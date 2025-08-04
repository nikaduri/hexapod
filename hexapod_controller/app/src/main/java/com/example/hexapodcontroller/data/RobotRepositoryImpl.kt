package com.example.hexapodcontroller.data

import android.content.SharedPreferences
import android.util.Log
import com.example.hexapodcontroller.domain.RobotRepository
import kotlinx.coroutines.*
import kotlinx.coroutines.flow.*
import java.io.IOException
import java.net.InetSocketAddress
import java.net.Socket
import java.util.concurrent.atomic.AtomicBoolean
import java.util.concurrent.atomic.AtomicLong
import javax.inject.Inject
import javax.inject.Singleton
import kotlinx.coroutines.coroutineScope

@Singleton
class RobotRepositoryImpl @Inject constructor(
    private val sharedPreferences: SharedPreferences,
    private val ioDispatcher: CoroutineDispatcher
) : RobotRepository {

    private val _connectionState = MutableStateFlow<RobotConnectionState>(RobotConnectionState.Disconnected)
    override val connectionState: Flow<RobotConnectionState> = _connectionState.asStateFlow()

    private var socket: Socket? = null
    private var continuousCommandJob: Job? = null
    private val isConnected = AtomicBoolean(false)
    private var lastCommandTime = AtomicLong(0)
    private var lastCommand: RobotCommand? = null

    override suspend fun connect(ipAddress: String, port: Int) {
        withContext(ioDispatcher) {
            try {
                _connectionState.value = RobotConnectionState.Connecting
                
                // Close existing connection if any
                disconnect()
                
                Log.d("RobotRepository", "Attempting to connect to $ipAddress:$port")
                val newSocket = Socket()
                try {
                    newSocket.connect(InetSocketAddress(ipAddress, port), SOCKET_TIMEOUT)
                } catch (e: Exception) {
                    _connectionState.value = RobotConnectionState.Error("Failed to connect: Connection refused")
                    Log.e("RobotRepository", "Connection failed: ${e.message}", e)
                    return@withContext
                }
                
                // Configure socket options for better stability
                newSocket.soTimeout = SOCKET_TIMEOUT
                newSocket.keepAlive = true
                newSocket.tcpNoDelay = true // Disable Nagle's algorithm
                newSocket.sendBufferSize = 512  // Reduced for faster transmission
                newSocket.receiveBufferSize = 512  // Reduced for faster reception
                newSocket.setPerformancePreferences(0, 1, 0) // Prioritize latency over bandwidth
                
                socket = newSocket
                isConnected.set(true)
                
                _connectionState.value = RobotConnectionState.Connected(ipAddress, port)
                Log.d("RobotRepository", "Successfully connected to $ipAddress:$port")
                
                // Start connection monitoring in a new coroutine (less aggressive)
                launch {
                    monitorConnection()
                }
            } catch (e: Exception) {
                val errorMsg = when {
                    e.message?.contains("refused") == true -> "Connection refused: Robot not available"
                    e.message?.contains("timeout") == true -> "Connection timeout: Robot not responding"
                    else -> "Failed to connect: ${e.message}"
                }
                _connectionState.value = RobotConnectionState.Error(errorMsg)
                Log.e("RobotRepository", "Connection error: ${e.message}", e)
                disconnect()
            }
        }
    }

    override suspend fun disconnect() {
        withContext(ioDispatcher) {
            try {
                continuousCommandJob?.cancelAndJoin()
                socket?.close()
            } catch (e: Exception) {
                // Ignore close errors
            } finally {
                socket = null
                isConnected.set(false)
                lastCommand = null
                _connectionState.value = RobotConnectionState.Disconnected
            }
        }
    }

    override suspend fun sendCommand(command: RobotCommand) {
        withContext(ioDispatcher) {
            try {
                if (!isConnected.get()) {
                    Log.e("RobotRepository", "Cannot send command: Not connected")
                    return@withContext
                }
                
                // Don't send the same command too frequently
                val now = System.currentTimeMillis()
                if (command == lastCommand && now - lastCommandTime.get() < MIN_COMMAND_INTERVAL) {
                    return@withContext
                }
                
                Log.d("RobotRepository", "Sending command: ${command.value}")
                socket?.getOutputStream()?.apply {
                    write(command.value.toByteArray())
                    flush()
                }
                
                lastCommand = command
                lastCommandTime.set(now)
            } catch (e: Exception) {
                Log.e("RobotRepository", "Failed to send command: ${e.message}", e)
                // Only disconnect if the socket is actually closed or we get a serious error
                if (socket?.isClosed == true || e is IOException) {
                    _connectionState.value = RobotConnectionState.Error("Connection lost")
                    disconnect()
                }
            }
        }
    }

    override suspend fun startContinuousCommand(command: RobotCommand) {
        Log.d("RobotRepository", "Starting continuous command: ${command.value}")
        stopContinuousCommand()
        continuousCommandJob = CoroutineScope(ioDispatcher).launch {
            try {
                while (isActive && isConnected.get()) {
                    sendCommand(command)
                    delay(CONTINUOUS_COMMAND_INTERVAL)
                }
            } catch (e: Exception) {
                Log.e("RobotRepository", "Continuous command failed: ${e.message}", e)
                // Don't disconnect on continuous command failure, just stop the job
            }
        }
    }

    override suspend fun stopContinuousCommand() {
        Log.d("RobotRepository", "Stopping continuous command")
        continuousCommandJob?.cancelAndJoin()
        continuousCommandJob = null
        // Don't send STOP command when stopping continuous commands
        // The movement commands should handle their own stopping
    }

    override fun getLastIpAddress(): String {
        return sharedPreferences.getString(KEY_IP_ADDRESS, DEFAULT_IP_ADDRESS) ?: DEFAULT_IP_ADDRESS
    }

    override fun getLastPort(): Int {
        return sharedPreferences.getInt(KEY_PORT, DEFAULT_PORT)
    }

    override fun saveConnectionSettings(ipAddress: String, port: Int) {
        sharedPreferences.edit().apply {
            putString(KEY_IP_ADDRESS, ipAddress)
            putInt(KEY_PORT, port)
            apply()
        }
    }

    private suspend fun monitorConnection() = coroutineScope {
        try {
            var isMonitoring = true
            var failedPings = 0
            
            while (isActive && isConnected.get() && isMonitoring) {
                val currentSocket = socket
                if (currentSocket == null || currentSocket.isClosed) {
                    Log.d("RobotRepository", "Socket is null or closed, stopping monitoring")
                    _connectionState.value = RobotConnectionState.Error("Connection lost")
                    disconnect()
                    isMonitoring = false
                    continue
                }

                // Simple connection check - send ping and expect response
                try {
                    // Send ping command (single byte 0) as expected by the server
                    currentSocket.getOutputStream().apply {
                        write(PING_COMMAND)
                        flush()
                    }
                    
                    // Wait for ping response (single byte 0) with timeout
                    currentSocket.soTimeout = PING_TIMEOUT
                    val response = currentSocket.getInputStream().read()
                    if (response == 0) {
                        // Successful ping
                        failedPings = 0
                        Log.d("RobotRepository", "Connection health check passed")
                    } else {
                        failedPings++
                        Log.w("RobotRepository", "Unexpected ping response: $response (failed pings: $failedPings)")
                    }
                    
                    // Reset socket timeout for normal operations
                    currentSocket.soTimeout = SOCKET_TIMEOUT
                    
                } catch (e: IOException) {
                    failedPings++
                    Log.e("RobotRepository", "Connection health check failed (failed pings: $failedPings)", e)
                    
                    // Only disconnect after multiple failed pings
                    if (failedPings >= MAX_FAILED_PINGS) {
                        _connectionState.value = RobotConnectionState.Error("Connection lost")
                        disconnect()
                        isMonitoring = false
                    }
                } catch (e: Exception) {
                    Log.e("RobotRepository", "Connection monitoring error", e)
                    // Don't disconnect immediately, just log the error
                }
                
                delay(CONNECTION_CHECK_INTERVAL)
            }
        } catch (e: Exception) {
            Log.e("RobotRepository", "Connection monitoring failed", e)
            // Only disconnect if we're still connected
            if (isConnected.get()) {
                _connectionState.value = RobotConnectionState.Error("Connection monitoring failed: ${e.message}")
                disconnect()
            }
        }
    }

    companion object {
        private const val SOCKET_TIMEOUT = 5000  // Reduced to 5 seconds for faster response
        private const val PING_TIMEOUT = 2000  // Reduced to 2 seconds for faster ping
        private const val CONNECTION_CHECK_INTERVAL = 60000L  // Check connection every 60 seconds (less frequent for speed)
        private const val CONTINUOUS_COMMAND_INTERVAL = 100L  // Reduced to 100ms for faster response
        private const val MIN_COMMAND_INTERVAL = 20L  // Reduced to 20ms for faster command sending
        private const val MAX_FAILED_PINGS = 5  // Allow 5 failed pings before disconnecting (more tolerant)
        private const val DEFAULT_IP_ADDRESS = "192.168.1.1"
        private const val DEFAULT_PORT = 8080
        private const val KEY_IP_ADDRESS = "ip_address"
        private const val KEY_PORT = "port"
        private val PING_COMMAND = byteArrayOf(0)  // Single byte ping as expected by server
    }
} 
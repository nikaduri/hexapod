package com.example.hexapodcontroller.domain

import com.example.hexapodcontroller.data.RobotCommand
import com.example.hexapodcontroller.data.RobotConnectionState
import com.example.hexapodcontroller.data.BatteryStatus
import kotlinx.coroutines.flow.Flow

interface RobotRepository {
    val connectionState: Flow<RobotConnectionState>
    val batteryStatus: Flow<BatteryStatus?>
    
    suspend fun connect(ipAddress: String, port: Int)
    suspend fun disconnect()
    suspend fun sendCommand(command: RobotCommand)
    suspend fun startContinuousCommand(command: RobotCommand)
    suspend fun stopContinuousCommand()
    suspend fun requestBatteryStatus(): BatteryStatus?
    
    fun getLastIpAddress(): String
    fun getLastPort(): Int
    fun saveConnectionSettings(ipAddress: String, port: Int)
} 
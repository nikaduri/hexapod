package com.example.hexapodcontroller.ui

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.example.hexapodcontroller.data.RobotCommand
import com.example.hexapodcontroller.data.RobotConnectionState
import com.example.hexapodcontroller.data.BatteryStatus
import com.example.hexapodcontroller.domain.RobotRepository
import dagger.hilt.android.lifecycle.HiltViewModel
import kotlinx.coroutines.flow.*
import kotlinx.coroutines.launch
import kotlinx.coroutines.delay
import kotlinx.coroutines.Job
import javax.inject.Inject

@HiltViewModel
class MainViewModel @Inject constructor(
    private val repository: RobotRepository
) : ViewModel() {

    private val _uiState = MutableStateFlow(MainUiState())
    val uiState: StateFlow<MainUiState> = _uiState.asStateFlow()
    
    val batteryStatus: StateFlow<BatteryStatus?> = repository.batteryStatus.stateIn(
        scope = viewModelScope,
        started = SharingStarted.WhileSubscribed(5000),
        initialValue = null
    )
    
    private var batteryPollingJob: Job? = null

    init {
        viewModelScope.launch {
            repository.connectionState.collect { state ->
                _uiState.update { it.copy(connectionState = state) }
                
                // Start or stop battery polling based on connection state
                when (state) {
                    is RobotConnectionState.Connected -> startBatteryPolling()
                    else -> stopBatteryPolling()
                }
            }
        }
    }

    fun onConnectClick() {
        viewModelScope.launch {
            repository.connect(repository.getLastIpAddress(), repository.getLastPort())
        }
    }

    fun onDisconnectClick() {
        viewModelScope.launch {
            repository.disconnect()
        }
    }

    fun onCommandButtonPressed(command: RobotCommand) {
        viewModelScope.launch {
            if (uiState.value.connectionState is RobotConnectionState.Connected) {
                repository.startContinuousCommand(command)
            }
        }
    }

    fun onCommandButtonReleased() {
        viewModelScope.launch {
            if (uiState.value.connectionState is RobotConnectionState.Connected) {
                repository.stopContinuousCommand()
            }
        }
    }

    fun onSingleCommand(command: RobotCommand) {
        viewModelScope.launch {
            if (uiState.value.connectionState is RobotConnectionState.Connected) {
                repository.sendCommand(command)
            }
        }
    }

    // Gait mode selection methods
    fun setTripodGait() {
        onSingleCommand(RobotCommand.TRIPOD_GAIT)
        _uiState.update { it.copy(currentGaitMode = GaitMode.TRIPOD) }
    }

    fun setWaveGait() {
        onSingleCommand(RobotCommand.WAVE_GAIT)
        _uiState.update { it.copy(currentGaitMode = GaitMode.WAVE) }
    }

    fun setRippleGait() {
        onSingleCommand(RobotCommand.RIPPLE_GAIT)
        _uiState.update { it.copy(currentGaitMode = GaitMode.RIPPLE) }
    }

    fun setStaircaseMode() {
        onSingleCommand(RobotCommand.STAIRCASE_MODE)
        _uiState.update { it.copy(currentGaitMode = GaitMode.STAIRCASE) }
    }

    fun updateConnectionSettings(ipAddress: String, port: Int) {
        repository.saveConnectionSettings(ipAddress, port)
    }

    fun getLastIpAddress(): String = repository.getLastIpAddress()

    fun getLastPort(): Int = repository.getLastPort()

    private fun startBatteryPolling() {
        stopBatteryPolling() // Stop any existing polling
        
        batteryPollingJob = viewModelScope.launch {
            // Initial battery request
            repository.requestBatteryStatus()
            
            // Poll every minute (60 seconds)
            while (true) {
                delay(60_000) // 60 seconds
                repository.requestBatteryStatus()
            }
        }
    }
    
    private fun stopBatteryPolling() {
        batteryPollingJob?.cancel()
        batteryPollingJob = null
    }

    override fun onCleared() {
        super.onCleared()
        stopBatteryPolling()
        viewModelScope.launch {
            repository.disconnect()
        }
    }
}

enum class GaitMode {
    TRIPOD, WAVE, RIPPLE, STAIRCASE
}

data class MainUiState(
    val connectionState: RobotConnectionState = RobotConnectionState.Disconnected,
    val isLoading: Boolean = false,
    val error: String? = null,
    val currentGaitMode: GaitMode = GaitMode.TRIPOD
) 
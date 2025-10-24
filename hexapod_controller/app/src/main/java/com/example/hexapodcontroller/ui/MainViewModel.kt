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

    fun setTripodGait() {
        onSingleCommand(RobotCommand.TRIPOD_GAIT)
        _uiState.update { it.copy(currentGaitMode = GaitMode.TRIPOD) }
    }

    fun setWaveGait() {
        onSingleCommand(RobotCommand.WAVE_GAIT)
        _uiState.update { it.copy(currentGaitMode = GaitMode.WAVE) }
    }

    fun updateConnectionSettings(ipAddress: String, port: Int) {
        repository.saveConnectionSettings(ipAddress, port)
    }

    fun getLastIpAddress(): String = repository.getLastIpAddress()

    fun getLastPort(): Int = repository.getLastPort()

    private fun startBatteryPolling() {
        stopBatteryPolling()
        
        batteryPollingJob = viewModelScope.launch {
            repository.requestBatteryStatus()
            
            while (true) {
                delay(60_000)
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
    TRIPOD, WAVE
}

data class MainUiState(
    val connectionState: RobotConnectionState = RobotConnectionState.Disconnected,
    val isLoading: Boolean = false,
    val error: String? = null,
    val currentGaitMode: GaitMode = GaitMode.TRIPOD
) 
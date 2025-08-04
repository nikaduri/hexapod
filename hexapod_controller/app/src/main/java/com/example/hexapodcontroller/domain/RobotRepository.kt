package com.example.hexapodcontroller.domain

import com.example.hexapodcontroller.data.RobotCommand
import com.example.hexapodcontroller.data.RobotConnectionState
import kotlinx.coroutines.flow.Flow

interface RobotRepository {
    val connectionState: Flow<RobotConnectionState>
    
    suspend fun connect(ipAddress: String, port: Int)
    suspend fun disconnect()
    suspend fun sendCommand(command: RobotCommand)
    suspend fun startContinuousCommand(command: RobotCommand)
    suspend fun stopContinuousCommand()
    
    fun getLastIpAddress(): String
    fun getLastPort(): Int
    fun saveConnectionSettings(ipAddress: String, port: Int)
} 
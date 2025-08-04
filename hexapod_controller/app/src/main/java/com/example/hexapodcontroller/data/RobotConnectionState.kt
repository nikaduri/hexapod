package com.example.hexapodcontroller.data

sealed class RobotConnectionState {
    object Disconnected : RobotConnectionState()
    object Connecting : RobotConnectionState()
    data class Connected(val ipAddress: String, val port: Int) : RobotConnectionState()
    data class Error(val message: String) : RobotConnectionState()
} 
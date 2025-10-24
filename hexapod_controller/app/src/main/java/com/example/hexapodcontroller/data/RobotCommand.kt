package com.example.hexapodcontroller.data

enum class RobotCommand(val value: String) {
    FORWARD("FORWARD"),
    BACKWARD("BACKWARD"),
    LEFT("LEFT"),
    RIGHT("RIGHT"),
    STOP("STOP"),
    STAND("STAND"),
    LAY_DOWN("LAY_DOWN"),
    DANCE("DANCE"),
    TRIPOD_GAIT("TRIPOD_GAIT"),
    WAVE_GAIT("WAVE_GAIT"),
    GET_BATTERY("GET_BATTERY");

    companion object {
        fun fromString(value: String): RobotCommand? =
            values().find { it.value == value.uppercase() }
    }
} 
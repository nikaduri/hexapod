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
    // Gait modes
    TRIPOD_GAIT("TRIPOD_GAIT"),
    WAVE_GAIT("WAVE_GAIT"),
    RIPPLE_GAIT("RIPPLE_GAIT"),
    STAIRCASE_MODE("STAIRCASE_MODE");

    companion object {
        fun fromString(value: String): RobotCommand? =
            values().find { it.value == value.uppercase() }
    }
} 
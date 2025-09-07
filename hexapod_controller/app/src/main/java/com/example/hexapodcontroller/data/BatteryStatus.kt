package com.example.hexapodcontroller.data

data class BatteryStatus(
    val percentage: Int,
    val lastUpdated: Long = System.currentTimeMillis()
)
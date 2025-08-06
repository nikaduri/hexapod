package com.example.hexapodcontroller.data

import kotlin.math.round

data class BatteryStatus(
    val voltage: Float,
    val percentage: Int,
    val lastUpdated: Long = System.currentTimeMillis()
) {
    companion object {
        // Voltage mapping for 2S battery (8.2V = 100%)
        private val voltagePercentageMap = mapOf(
            8.20f to 100,
            8.00f to 90,
            7.80f to 80,
            7.60f to 70,
            7.40f to 60,
            7.20f to 50,
            7.00f to 40,
            6.80f to 30,
            6.60f to 20,
            6.40f to 10,
            6.00f to 0
        )

        fun fromVoltage(voltage: Float): BatteryStatus {
            val percentage = calculatePercentageFromVoltage(voltage)
            return BatteryStatus(voltage, percentage)
        }

        private fun calculatePercentageFromVoltage(voltage: Float): Int {
            // Handle edge cases
            if (voltage >= 8.20f) return 100
            if (voltage <= 6.00f) return 0

            // Find the range that contains this voltage
            val sortedVoltages = voltagePercentageMap.keys.sortedDescending()
            
            for (i in 0 until sortedVoltages.size - 1) {
                val upperVoltage = sortedVoltages[i]
                val lowerVoltage = sortedVoltages[i + 1]
                
                if (voltage <= upperVoltage && voltage >= lowerVoltage) {
                    val upperPercentage = voltagePercentageMap[upperVoltage]!!
                    val lowerPercentage = voltagePercentageMap[lowerVoltage]!!
                    
                    // Linear interpolation between the two points
                    val voltageRange = upperVoltage - lowerVoltage
                    val percentageRange = upperPercentage - lowerPercentage
                    val voltageOffset = voltage - lowerVoltage
                    
                    val interpolatedPercentage = lowerPercentage + (voltageOffset / voltageRange) * percentageRange
                    return round(interpolatedPercentage).toInt()
                }
            }
            
            // Fallback (shouldn't reach here)
            return 0
        }
    }
}
package com.example.hexapodcontroller.ui

import android.content.Context
import android.util.AttributeSet
import android.view.LayoutInflater
import android.widget.ImageView
import android.widget.LinearLayout
import android.widget.TextView
import com.example.hexapodcontroller.R
import com.example.hexapodcontroller.data.BatteryStatus

class BatteryStatusView @JvmOverloads constructor(
    context: Context,
    attrs: AttributeSet? = null,
    defStyleAttr: Int = 0
) : LinearLayout(context, attrs, defStyleAttr) {

    private val batteryIcon: ImageView
    private val batteryPercentage: TextView

    init {
        LayoutInflater.from(context).inflate(R.layout.battery_status_view, this, true)
        
        batteryIcon = findViewById(R.id.iv_battery_icon)
        batteryPercentage = findViewById(R.id.tv_battery_percentage)
        
        // Set default state
        updateBatteryDisplay(null)
    }

    fun updateBatteryStatus(batteryStatus: BatteryStatus?) {
        updateBatteryDisplay(batteryStatus)
    }

    private fun updateBatteryDisplay(batteryStatus: BatteryStatus?) {
        if (batteryStatus == null) {
            batteryPercentage.text = "--"
            batteryIcon.setImageResource(R.drawable.ic_battery)
            alpha = 0.5f
            return
        }

        alpha = 1.0f
        batteryPercentage.text = "${batteryStatus.percentage}%"

        // Update icon based on percentage
        val iconRes = when {
            batteryStatus.percentage >= 60 -> R.drawable.ic_battery_full
            batteryStatus.percentage >= 30 -> R.drawable.ic_battery_medium
            else -> R.drawable.ic_battery_low
        }
        batteryIcon.setImageResource(iconRes)
    }
}
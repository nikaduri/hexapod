package com.example.hexapodcontroller

import android.os.Bundle
import android.text.InputFilter
import android.text.InputType
import android.text.Spanned
import android.widget.EditText
import android.widget.Toast
import androidx.activity.viewModels
import androidx.appcompat.app.AppCompatActivity
import com.example.hexapodcontroller.databinding.ActivitySettingsBinding
import com.example.hexapodcontroller.ui.MainViewModel
import com.google.android.material.button.MaterialButton
import dagger.hilt.android.AndroidEntryPoint

@AndroidEntryPoint
class SettingsActivity : AppCompatActivity() {

    private lateinit var binding: ActivitySettingsBinding
    private lateinit var etIpAddress: EditText
    private lateinit var etPort: EditText
    private lateinit var btnSave: MaterialButton
    private val viewModel: MainViewModel by viewModels()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivitySettingsBinding.inflate(layoutInflater)
        setContentView(binding.root)

        etIpAddress = binding.etIpAddress
        etPort = binding.etPort
        btnSave = binding.btnSave

        // Configure IP address input
        etIpAddress.apply {
            inputType = InputType.TYPE_CLASS_NUMBER or InputType.TYPE_CLASS_TEXT
            filters = arrayOf(IpAddressInputFilter())
        }

        // Configure port input to only accept numbers
        etPort.inputType = InputType.TYPE_CLASS_NUMBER

        // Check for scanned values from QR code
        val scannedIp = intent.getStringExtra("SCANNED_IP")
        val scannedPort = intent.getIntExtra("SCANNED_PORT", -1)

        if (scannedIp != null && scannedPort != -1) {
            // Use scanned values
            etIpAddress.setText(scannedIp)
            etPort.setText(scannedPort.toString())
        } else {
            // Load current settings
            etIpAddress.setText(viewModel.getLastIpAddress())
            etPort.setText(viewModel.getLastPort().toString())
        }

        btnSave.setOnClickListener {
            saveSettings()
        }
    }

    private fun saveSettings() {
        val ipAddress = etIpAddress.text.toString()
        val portStr = etPort.text.toString()

        if (ipAddress.isEmpty() || portStr.isEmpty()) {
            Toast.makeText(this, "Please fill in all fields", Toast.LENGTH_SHORT).show()
            return
        }

        try {
            val port = portStr.toInt()
            if (port !in 1..65535) {
                Toast.makeText(this, "Port must be between 1 and 65535", Toast.LENGTH_SHORT).show()
                return
            }

            // Validate IP address format
            if (!isValidIpAddress(ipAddress)) {
                Toast.makeText(this, "Invalid IP address format", Toast.LENGTH_SHORT).show()
                return
            }

            viewModel.updateConnectionSettings(ipAddress, port)
            Toast.makeText(this, "Settings saved", Toast.LENGTH_SHORT).show()
            finish()
        } catch (e: NumberFormatException) {
            Toast.makeText(this, "Invalid port number", Toast.LENGTH_SHORT).show()
        }
    }

    private fun isValidIpAddress(ip: String): Boolean {
        return try {
            val parts = ip.split(".")
            if (parts.size != 4) return false
            parts.all { it.toInt() in 0..255 }
        } catch (e: Exception) {
            false
        }
    }

    private class IpAddressInputFilter : InputFilter {
        override fun filter(
            source: CharSequence?,
            start: Int,
            end: Int,
            dest: Spanned?,
            dstart: Int,
            dend: Int
        ): CharSequence? {
            if (source == null || dest == null) return null

            val builder = StringBuilder()
            for (i in start until end) {
                val c = source[i]
                if (c.isDigit() || c == '.') {
                    builder.append(c)
                }
            }

            // If nothing has changed, return null to accept the original input
            if (builder.toString() == source.toString()) {
                return null
            }

            // Return the filtered string
            return builder.toString()
        }
    }
}
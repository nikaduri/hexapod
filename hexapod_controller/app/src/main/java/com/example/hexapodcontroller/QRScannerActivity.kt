package com.example.hexapodcontroller

import android.Manifest
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Bundle
import android.widget.Toast
import androidx.activity.viewModels
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import com.budiyev.android.codescanner.*
import com.example.hexapodcontroller.data.RobotCommand
import com.example.hexapodcontroller.ui.MainViewModel
import dagger.hilt.android.AndroidEntryPoint

@AndroidEntryPoint
class QRScannerActivity : AppCompatActivity() {
    private lateinit var codeScanner: CodeScanner
    private val viewModel: MainViewModel by viewModels()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_qrscanner)

        if (ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this, arrayOf(Manifest.permission.CAMERA), 101)
        } else {
            startScanner()
        }
    }

    private fun startScanner() {
        val scannerView = findViewById<CodeScannerView>(R.id.scanner_view)
        codeScanner = CodeScanner(this, scannerView)

        // Scanner configuration
        codeScanner.apply {
            camera = CodeScanner.CAMERA_BACK
            formats = CodeScanner.ALL_FORMATS
            autoFocusMode = AutoFocusMode.SAFE
            scanMode = ScanMode.SINGLE
            isAutoFocusEnabled = true
            isFlashEnabled = false

            // Handle scanned QR codes
            decodeCallback = DecodeCallback {
                runOnUiThread {
                    processQRCode(it.text)
                }
            }

            errorCallback = ErrorCallback {
                runOnUiThread {
                    Toast.makeText(this@QRScannerActivity,
                        "Camera error: ${it.message}",
                        Toast.LENGTH_LONG).show()
                }
            }
        }

        scannerView.setOnClickListener {
            codeScanner.startPreview()
        }
    }

    private fun processQRCode(content: String) {
        // Debug logging
        android.util.Log.d("QRScanner", "Raw content: '$content'")
        
        // Clean the content by removing common URL prefixes and whitespace
        var cleanContent = content.trim()
            .replace(Regex("^(http://|https://|tcp://|)"), "")
            .replace(Regex("/.*$"), "") // Remove anything after a slash
        
        android.util.Log.d("QRScanner", "Cleaned content: '$cleanContent'")

        // First try to parse as a robot command
        val command = RobotCommand.fromString(cleanContent)
        if (command != null) {
            viewModel.onSingleCommand(command)
            Toast.makeText(this, "Executing: ${command.value}", Toast.LENGTH_SHORT).show()
            finish()
            return
        }

        // Not a command, try to parse as IP:Port
        try {
            // Find the last colon in the string
            val colonIndex = cleanContent.lastIndexOf(':')
            if (colonIndex == -1) {
                // If no port specified, assume default port 8080
                cleanContent += ":8080"
                android.util.Log.d("QRScanner", "No port specified, using default: '$cleanContent'")
            }

            val parts = cleanContent.split(":")
            val ipAddress = parts[0].trim()
            val portStr = parts[1].trim()
            
            android.util.Log.d("QRScanner", "Parsed - IP: '$ipAddress', Port: '$portStr'")

            // Validate IP address
            if (!ipAddress.matches(Regex("^\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}$"))) {
                Toast.makeText(this, "Invalid IP address format: $ipAddress", Toast.LENGTH_SHORT).show()
                return
            }

            // Validate port
            val port = portStr.toInt()
            if (port !in 1..65535) {
                Toast.makeText(this, "Port must be between 1 and 65535", Toast.LENGTH_SHORT).show()
                return
            }

            // All validation passed, proceed with the connection
            val intent = Intent(this, SettingsActivity::class.java).apply {
                putExtra("SCANNED_IP", ipAddress)
                putExtra("SCANNED_PORT", port)
            }
            startActivity(intent)
            finish()
        } catch (e: NumberFormatException) {
            Toast.makeText(this, "Invalid port number", Toast.LENGTH_SHORT).show()
        } catch (e: Exception) {
            android.util.Log.e("QRScanner", "Error processing QR code", e)
            Toast.makeText(this, "Error: ${e.message}", Toast.LENGTH_SHORT).show()
        }
    }

    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<out String>,
        grantResults: IntArray
    ) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        if (requestCode == 101 && grantResults.isNotEmpty() && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
            startScanner()
        } else {
            Toast.makeText(this, "Camera permission required", Toast.LENGTH_LONG).show()
            finish()
        }
    }

    override fun onResume() {
        super.onResume()
        if (::codeScanner.isInitialized) {
            codeScanner.startPreview()
        }
    }

    override fun onPause() {
        if (::codeScanner.isInitialized) {
            codeScanner.releaseResources()
        }
        super.onPause()
    }
}
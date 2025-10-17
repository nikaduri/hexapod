package com.example.hexapodcontroller

import android.Manifest
import android.annotation.SuppressLint
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Bundle
import android.util.Log
import android.view.Menu
import android.view.MenuItem
import android.view.MotionEvent
import android.view.View
import android.widget.Button
import android.widget.TextView
import android.widget.Toast
import androidx.activity.viewModels
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import androidx.core.view.GravityCompat
import androidx.drawerlayout.widget.DrawerLayout
import androidx.lifecycle.Lifecycle
import androidx.lifecycle.lifecycleScope
import androidx.lifecycle.repeatOnLifecycle
import com.example.hexapodcontroller.data.RobotCommand
import com.example.hexapodcontroller.data.RobotConnectionState
import com.example.hexapodcontroller.databinding.ActivityMainBinding
import com.example.hexapodcontroller.ui.MainViewModel
import com.example.hexapodcontroller.ui.GaitMode
import com.google.android.material.navigation.NavigationView
import dagger.hilt.android.AndroidEntryPoint
import kotlinx.coroutines.launch

@AndroidEntryPoint
class MainActivity : AppCompatActivity() {

    private val viewModel: MainViewModel by viewModels()
    private lateinit var binding: ActivityMainBinding

    companion object {
        private const val CAMERA_PERMISSION_REQUEST = 100
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        setupClickListeners()
        setupToolbar()
        setupNavigationDrawer()
        observeState()
    }

    @SuppressLint("ClickableViewAccessibility")
    private fun setupClickListeners() {
        // Movement controls with continuous transmission
        val motionButtonListener = View.OnTouchListener { v, event ->
            when (event.action) {
                MotionEvent.ACTION_DOWN -> {
                    v.isPressed = true  // Manually set pressed state
                    when (v.id) {
                        R.id.btn_forward -> {
                            Log.d("MainActivity", "Forward button pressed")
                            viewModel.onCommandButtonPressed(RobotCommand.FORWARD)
                        }
                        R.id.btn_backward -> {
                            Log.d("MainActivity", "Backward button pressed")
                            viewModel.onCommandButtonPressed(RobotCommand.BACKWARD)
                        }
                        R.id.btn_left -> {
                            Log.d("MainActivity", "Left button pressed")
                            viewModel.onCommandButtonPressed(RobotCommand.LEFT)
                        }
                        R.id.btn_right -> {
                            Log.d("MainActivity", "Right button pressed")
                            viewModel.onCommandButtonPressed(RobotCommand.RIGHT)
                        }
                    }
                }
                MotionEvent.ACTION_UP, MotionEvent.ACTION_CANCEL -> {
                    v.isPressed = false  // Reset pressed state
                    Log.d("MainActivity", "Button released")
                    viewModel.onCommandButtonReleased()
                }
            }
            false  // Return false to allow the button's default touch handling
        }

        with(binding) {
            btnForward.setOnTouchListener(motionButtonListener)
            btnBackward.setOnTouchListener(motionButtonListener)
            btnLeft.setOnTouchListener(motionButtonListener)
            btnRight.setOnTouchListener(motionButtonListener)

            // Single command buttons
            btnStop.setOnClickListener { 
                Log.d("MainActivity", "Stop button clicked")
                viewModel.onSingleCommand(RobotCommand.STOP) 
            }
            btnStand.setOnClickListener { 
                Log.d("MainActivity", "Stand button clicked")
                viewModel.onSingleCommand(RobotCommand.STAND) 
            }
            btnLayDown.setOnClickListener { 
                Log.d("MainActivity", "Lay Down button clicked")
                viewModel.onSingleCommand(RobotCommand.LAY_DOWN) 
            }
            btnDance.setOnClickListener { 
                Log.d("MainActivity", "Dance button clicked")
                viewModel.onSingleCommand(RobotCommand.DANCE) 
            }

            // Gait mode buttons
            btnTripodGait.setOnClickListener {
                Log.d("MainActivity", "Tripod Gait button clicked")
                viewModel.setTripodGait()
            }
            btnWaveGait.setOnClickListener {
                Log.d("MainActivity", "Wave Gait button clicked")
                viewModel.setWaveGait()
            }
            btnRippleGait.setOnClickListener {
                Log.d("MainActivity", "Ripple Gait button clicked")
                viewModel.setRippleGait()
            }
            btnStaircaseMode.setOnClickListener {
                Log.d("MainActivity", "Staircase Mode button clicked")
                viewModel.setStaircaseMode()
            }

            // QR Scanner launch
            //btnScanQr.setOnClickListener { checkCameraPermission() }
        }
    }

    private fun setupToolbar() {
        setSupportActionBar(binding.toolbar)
        supportActionBar?.setDisplayHomeAsUpEnabled(true)
        supportActionBar?.setHomeAsUpIndicator(R.drawable.ic_menu)
    }

    private fun setupNavigationDrawer() {
        binding.navView.setNavigationItemSelectedListener { item ->
            when (item.itemId) {
                R.id.nav_home -> { /* Already on home */ }
                R.id.nav_settings -> {
                    startActivity(Intent(this, SettingsActivity::class.java))
                }
                R.id.nav_connect -> {
                    viewModel.onConnectClick()
                }
                R.id.nav_disconnect -> {
                    viewModel.onDisconnectClick()
                }
                R.id.nav_qr -> {
                    checkCameraPermission()
                }
                R.id.nav_info -> {
                    showInfoDialog()
                }
            }
            binding.drawerLayout.closeDrawer(GravityCompat.START)
            true
        }
    }

    private fun observeState() {
        lifecycleScope.launch {
            repeatOnLifecycle(Lifecycle.State.STARTED) {
                viewModel.uiState.collect { state ->
                    updateUI(state.connectionState)
                    updateGaitModeHighlighting(state.currentGaitMode)
                    state.error?.let { showError(it) }
                }
            }
        }
        
        // Observe battery status separately
        lifecycleScope.launch {
            repeatOnLifecycle(Lifecycle.State.STARTED) {
                viewModel.batteryStatus.collect { batteryStatus ->
                    binding.batteryStatusView.updateBatteryStatus(batteryStatus)
                }
            }
        }
    }

    private fun updateUI(connectionState: RobotConnectionState) {
        val statusText = when (connectionState) {
            is RobotConnectionState.Connected -> "Connected to ${connectionState.ipAddress}:${connectionState.port}"
            is RobotConnectionState.Connecting -> "Connecting..."
            is RobotConnectionState.Disconnected -> "Disconnected"
            is RobotConnectionState.Error -> "Error: ${connectionState.message}"
        }
        binding.tvStatus.text = statusText
        
        // Log connection state changes
        Log.d("MainActivity", "Connection state changed to: $connectionState")
        if (connectionState is RobotConnectionState.Error) {
            Log.e("MainActivity", "Connection error: ${connectionState.message}")
        }
    }

    private fun updateGaitModeHighlighting(currentGaitMode: GaitMode) {
        // Reset all gait buttons to normal style
        binding.btnTripodGait.setBackgroundTintList(null)
        binding.btnWaveGait.setBackgroundTintList(null)
        binding.btnRippleGait.setBackgroundTintList(null)
        binding.btnStaircaseMode.setBackgroundTintList(null)

        // Apply normal style to all buttons
        binding.btnTripodGait.setBackgroundTintList(android.content.res.ColorStateList.valueOf(resources.getColor(R.color.accent, null)))
        binding.btnWaveGait.setBackgroundTintList(android.content.res.ColorStateList.valueOf(resources.getColor(R.color.accent, null)))
        binding.btnRippleGait.setBackgroundTintList(android.content.res.ColorStateList.valueOf(resources.getColor(R.color.accent, null)))
        binding.btnStaircaseMode.setBackgroundTintList(android.content.res.ColorStateList.valueOf(resources.getColor(R.color.accent, null)))

        // Highlight the current gait mode with distinct color
        when (currentGaitMode) {
            GaitMode.TRIPOD -> binding.btnTripodGait.setBackgroundTintList(android.content.res.ColorStateList.valueOf(resources.getColor(R.color.gait_selected, null)))
            GaitMode.WAVE -> binding.btnWaveGait.setBackgroundTintList(android.content.res.ColorStateList.valueOf(resources.getColor(R.color.gait_selected, null)))
            GaitMode.RIPPLE -> binding.btnRippleGait.setBackgroundTintList(android.content.res.ColorStateList.valueOf(resources.getColor(R.color.gait_selected, null)))
            GaitMode.STAIRCASE -> binding.btnStaircaseMode.setBackgroundTintList(android.content.res.ColorStateList.valueOf(resources.getColor(R.color.gait_selected, null)))
        }
    }

    private fun showError(error: String) {
        Toast.makeText(this, error, Toast.LENGTH_SHORT).show()
    }

    private fun checkCameraPermission() {
        when {
            ContextCompat.checkSelfPermission(
                this,
                Manifest.permission.CAMERA
            ) == PackageManager.PERMISSION_GRANTED -> {
                launchQRScanner()
            }
            ActivityCompat.shouldShowRequestPermissionRationale(
                this,
                Manifest.permission.CAMERA
            ) -> {
                showPermissionRationale()
            }
            else -> {
                ActivityCompat.requestPermissions(
                    this,
                    arrayOf(Manifest.permission.CAMERA),
                    CAMERA_PERMISSION_REQUEST
                )
            }
        }
    }

    private fun showPermissionRationale() {
        AlertDialog.Builder(this)
            .setTitle("Camera Access Needed")
            .setMessage("QR scanning requires camera access")
            .setPositiveButton("Allow") { _, _ ->
                ActivityCompat.requestPermissions(
                    this,
                    arrayOf(Manifest.permission.CAMERA),
                    CAMERA_PERMISSION_REQUEST
                )
            }
            .setNegativeButton("Deny", null)
            .show()
    }

    private fun launchQRScanner() {
        startActivity(Intent(this, QRScannerActivity::class.java))
    }

    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<out String>,
        grantResults: IntArray
    ) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        when (requestCode) {
            CAMERA_PERMISSION_REQUEST -> {
                if (grantResults.isNotEmpty() && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    launchQRScanner()
                } else {
                    Toast.makeText(this, "Camera permission denied", Toast.LENGTH_SHORT).show()
                }
            }
        }
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        return when (item.itemId) {
            android.R.id.home -> {
                binding.drawerLayout.openDrawer(GravityCompat.START)
                true
            }
            else -> super.onOptionsItemSelected(item)
        }
    }

    private fun showInfoDialog() {
        AlertDialog.Builder(this)
            .setTitle("Hexapod Controller")
            .setMessage("Version 1.0\n\nUse the controls to command your hexapod robot.")
            .setPositiveButton("OK", null)
            .show()
    }
}
package com.example.mysnipeit

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.viewModels
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import com.example.mysnipeit.ui.theme.MySniperItTheme
import com.example.mysnipeit.viewmodel.SniperViewModel
import com.example.mysnipeit.viewmodel.AppScreen
import com.example.mysnipeit.ui.device.DeviceSelectionScreen
import com.example.mysnipeit.ui.dashboard.DashboardScreen

class MainActivity : ComponentActivity() {
    private val viewModel: SniperViewModel by viewModels()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            MySniperItTheme {
                Surface(
                    modifier = Modifier.fillMaxSize(),
                    color = MaterialTheme.colorScheme.background
                ) {
                    SniperApp(viewModel = viewModel)
                }
            }
        }
    }
}

@Composable
fun SniperApp(viewModel: SniperViewModel) {
    val uiState by viewModel.uiState.collectAsStateWithLifecycle()
    val availableDevices by viewModel.availableDevices.collectAsStateWithLifecycle()
    val sensorData by viewModel.sensorData.collectAsStateWithLifecycle()
    val detectedTargets by viewModel.detectedTargets.collectAsStateWithLifecycle()
    val shootingSolution by viewModel.shootingSolution.collectAsStateWithLifecycle()
    val systemStatus by viewModel.systemStatus.collectAsStateWithLifecycle()

    when (uiState.currentScreen) {
        AppScreen.DEVICE_SELECTION -> {
            DeviceSelectionScreen(
                devices = availableDevices,
                onDeviceSelected = { device ->
                    viewModel.selectDevice(device)
                },
                onScanClick = {
                    viewModel.scanForDevices()
                }
            )
        }

        AppScreen.DASHBOARD -> {
            DashboardScreen(
                sensorData = sensorData,
                detectedTargets = detectedTargets,
                shootingSolution = shootingSolution,
                systemStatus = systemStatus,
                onConnectClick = {
                    viewModel.connectToSystem()
                },
                onDisconnectClick = {
                    viewModel.disconnectFromSystem()
                },
                onBackClick = {
                    viewModel.goBackToDeviceSelection()
                },
                onMenuClick = {
                    // TODO: Implement menu functionality later
                    // For now, just log the click
                    android.util.Log.d("Dashboard", "Menu clicked")
                }
            )
        }
    }
}
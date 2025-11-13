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
import com.example.mysnipeit.ui.home.HomeScreen
import com.example.mysnipeit.ui.device.DeviceSelectionScreen
import com.example.mysnipeit.ui.map.MapScreen
import com.example.mysnipeit.ui.dashboard.DashboardScreen
import com.google.android.gms.maps.model.LatLng
import android.util.Log
import com.example.mysnipeit.ui.diagnostics.DiagnosticsScreen

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
    val selectedTargetId = uiState.selectedTargetId

    // User location (mock location for now - can be replaced with real GPS later)
    val userLocation = remember { LatLng( 31.518209, 34.521274) }

    when (uiState.currentScreen) {
        AppScreen.HOME -> {
            HomeScreen(
                onDeviceListClick = {
                    viewModel.navigateToDeviceList()
                },
                onMapClick = {
                    viewModel.navigateToMap()
                },
                onDiagnosticsClick = {
                    viewModel.navigateToDiagnostics() }
            )
        }

        AppScreen.DEVICE_SELECTION -> {
            DeviceSelectionScreen(
                devices = availableDevices,
                onDeviceSelected = { device ->
                    viewModel.selectDevice(device)
                },
                onBackClick = {
                    viewModel.navigateToHome()
                }
            )
        }

        AppScreen.MAP -> {
            MapScreen(
                devices = availableDevices,
                userLocation = userLocation,
                onDeviceSelected = { device ->
                    viewModel.selectDevice(device)
                },
                onBackClick = {
                    viewModel.navigateToHome()
                }
            )
        }

        AppScreen.DASHBOARD -> {
            DashboardScreen(
                sensorData = sensorData,
                detectedTargets = detectedTargets,
                shootingSolution = shootingSolution,
                systemStatus = systemStatus,
                selectedTargetId = selectedTargetId,
                onTargetSelect = { targetId ->
                    if (targetId.isEmpty()) {
                        viewModel.deselectTarget()
                    } else {
                        viewModel.selectTarget(targetId)
                    }
                },
                onConnectClick = { viewModel.connectToSystem() },
                onDisconnectClick = { viewModel.disconnectFromSystem() },
                onBackClick = { viewModel.goBackFromDashboard() },
                onMenuClick = { /* TODO */ }
            )
        }

        AppScreen.DIAGNOSTICS -> {
            DiagnosticsScreen(
                onBackClick = { viewModel.navigateToHome() }
            )
        }
    }
}
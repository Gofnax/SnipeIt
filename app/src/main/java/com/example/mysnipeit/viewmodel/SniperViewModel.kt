package com.example.mysnipeit.viewmodel

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.example.mysnipeit.data.models.*
import com.example.mysnipeit.data.network.RaspberryPiClient
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.launch
import android.util.Log

class SniperViewModel : ViewModel() {

    // Create client directly for now
    private val raspberryPiClient = RaspberryPiClient()

    // UI State
    private val _uiState = MutableStateFlow(SniperUiState())
    val uiState: StateFlow<SniperUiState> = _uiState.asStateFlow()

    // Mock devices
    private val _availableDevices = MutableStateFlow(
        listOf(
            Device(
                id = "device_1",
                name = "Device 1",
                location = "Sector A",
                longitude = 32.014,
                latitude = 35.093,
                status = DeviceStatus.INACTIVE,
                batteryLevel = 89,
                ipAddress = "192.168.1.100"
            ),
            Device(
                id = "device_4",
                name = "Device 4",
                location = "Sector A",
                longitude = 32.014,
                latitude = 35.093,
                status = DeviceStatus.ACTIVE,
                batteryLevel = 87,
                ipAddress = "192.168.1.104"
            )
        )
    )
    val availableDevices: StateFlow<List<Device>> = _availableDevices

    private val _selectedDevice = MutableStateFlow<Device?>(null)
    val selectedDevice: StateFlow<Device?> = _selectedDevice

    // Data from client - connect directly
    val sensorData: StateFlow<SensorData?> = raspberryPiClient.sensorData
    val detectedTargets: StateFlow<List<DetectedTarget>> = raspberryPiClient.detectedTargets
    val shootingSolution: StateFlow<ShootingSolution?> = raspberryPiClient.shootingSolution
    val systemStatus: StateFlow<SystemStatus> = raspberryPiClient.systemStatus

    fun selectDevice(device: Device) {
        Log.d("SniperViewModel", "selectDevice called for: ${device.name}")
        _selectedDevice.value = device
        _uiState.value = _uiState.value.copy(
            currentScreen = AppScreen.DASHBOARD,
            selectedDeviceId = device.id
        )
        // Start connection
        connectToDevice(device)
    }

    private fun connectToDevice(device: Device) {
        Log.d("SniperViewModel", "connectToDevice called for IP: ${device.ipAddress}")
        viewModelScope.launch {
            try {
                raspberryPiClient.connect(device.ipAddress)
                Log.d("SniperViewModel", "Connection initiated successfully")
            } catch (e: Exception) {
                Log.e("SniperViewModel", "Connection failed: ${e.message}")
                _uiState.value = _uiState.value.copy(
                    connectionError = "Connection failed: ${e.message}"
                )
            }
        }
    }

    fun scanForDevices() {
        _uiState.value = _uiState.value.copy(isScanning = true)

        viewModelScope.launch {
            kotlinx.coroutines.delay(3000)
            _uiState.value = _uiState.value.copy(isScanning = false)
        }
    }

    fun goBackToDeviceSelection() {
        Log.d("SniperViewModel", "goBackToDeviceSelection called")
        _uiState.value = _uiState.value.copy(currentScreen = AppScreen.DEVICE_SELECTION)
        _selectedDevice.value = null
        raspberryPiClient.disconnect()
    }

    fun connectToSystem() {
        _selectedDevice.value?.let { device ->
            connectToDevice(device)
        }
    }

    fun disconnectFromSystem() {
        raspberryPiClient.disconnect()
    }
}

data class SniperUiState(
    val currentScreen: AppScreen = AppScreen.DEVICE_SELECTION,
    val isScanning: Boolean = false,
    val selectedDeviceId: String? = null,
    val connectionError: String? = null,
    val isVideoFullscreen: Boolean = false
)

enum class AppScreen {
    DEVICE_SELECTION,
    DASHBOARD
}
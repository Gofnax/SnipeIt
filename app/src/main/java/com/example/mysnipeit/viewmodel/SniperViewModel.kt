package com.example.mysnipeit.viewmodel

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.example.mysnipeit.data.models.SensorData
import com.example.mysnipeit.data.models.DetectedTarget
import com.example.mysnipeit.data.models.ShootingSolution
import com.example.mysnipeit.data.models.SystemStatus
import com.example.mysnipeit.data.models.ConnectionState
import com.example.mysnipeit.data.models.Device
import com.example.mysnipeit.data.models.DeviceStatus
import com.example.mysnipeit.data.repository.SniperRepository
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.launch

class SniperViewModel : ViewModel() {

    private val repository = SniperRepository() // We'll inject this later

    // UI State
    private val _uiState = MutableStateFlow(SniperUiState())
    val uiState: StateFlow<SniperUiState> = _uiState.asStateFlow()

    // Mock devices for now (replace with real discovery later)
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

    // Data from repository (will be connected later)
    private val _sensorData = MutableStateFlow<SensorData?>(null)
    val sensorData: StateFlow<SensorData?> = _sensorData

    private val _detectedTargets = MutableStateFlow<List<DetectedTarget>>(emptyList())
    val detectedTargets: StateFlow<List<DetectedTarget>> = _detectedTargets

    private val _shootingSolution = MutableStateFlow<ShootingSolution?>(null)
    val shootingSolution: StateFlow<ShootingSolution?> = _shootingSolution

    private val _systemStatus = MutableStateFlow(
        SystemStatus(
            connectionStatus = ConnectionState.DISCONNECTED,
            batteryLevel = null,
            cameraStatus = false,
            gpsStatus = false,
            rangefinderStatus = false,
            microphoneStatus = false,
            lastHeartbeat = 0L
        )
    )
    val systemStatus: StateFlow<SystemStatus> = _systemStatus

    fun selectDevice(device: Device) {
        _selectedDevice.value = device
        _uiState.value = _uiState.value.copy(
            currentScreen = AppScreen.DASHBOARD,
            selectedDeviceId = device.id
        )
        // Start connection to the selected device
        connectToDevice(device)
    }

    private fun connectToDevice(device: Device) {
        viewModelScope.launch {
            _systemStatus.value = _systemStatus.value.copy(
                connectionStatus = ConnectionState.CONNECTING
            )

            // Simulate connection process
            try {
                // Replace with actual repository connection
                kotlinx.coroutines.delay(2000) // Simulate connection time
                _systemStatus.value = _systemStatus.value.copy(
                    connectionStatus = ConnectionState.CONNECTED,
                    batteryLevel = device.batteryLevel
                )
            } catch (e: Exception) {
                _systemStatus.value = _systemStatus.value.copy(
                    connectionStatus = ConnectionState.ERROR
                )
            }
        }
    }

    fun scanForDevices() {
        // Implement device scanning logic
        _uiState.value = _uiState.value.copy(isScanning = true)

        viewModelScope.launch {
            // Simulate scanning
            kotlinx.coroutines.delay(3000)
            _uiState.value = _uiState.value.copy(isScanning = false)
        }
    }

    fun goBackToDeviceSelection() {
        _uiState.value = _uiState.value.copy(currentScreen = AppScreen.DEVICE_SELECTION)
        _selectedDevice.value = null
        _systemStatus.value = _systemStatus.value.copy(connectionStatus = ConnectionState.DISCONNECTED)
    }

    fun connectToSystem() {
        _selectedDevice.value?.let { device ->
            connectToDevice(device)
        }
    }

    fun disconnectFromSystem() {
        _systemStatus.value = _systemStatus.value.copy(connectionStatus = ConnectionState.DISCONNECTED)
    }
}

// UI State data class
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
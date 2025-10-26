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

    private val raspberryPiClient = RaspberryPiClient()

    private val _uiState = MutableStateFlow(SniperUiState())
    val uiState: StateFlow<SniperUiState> = _uiState.asStateFlow()

    // All 4 devices
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
                id = "device_2",
                name = "Device 2",
                location = "Sector B",
                longitude = 32.016,
                latitude = 35.095,
                status = DeviceStatus.INACTIVE,
                batteryLevel = 72,
                ipAddress = "192.168.1.101"
            ),
            Device(
                id = "device_3",
                name = "Device 3",
                location = "Sector C",
                longitude = 32.012,
                latitude = 35.091,
                status = DeviceStatus.ACTIVE,
                batteryLevel = 95,
                ipAddress = "192.168.1.102"
            ),
            Device(
                id = "device_4",
                name = "Device 4",
                location = "Sector D",
                longitude = 32.018,
                latitude = 35.097,
                status = DeviceStatus.ACTIVE,
                batteryLevel = 87,
                ipAddress = "192.168.1.104"
            )
        )
    )
    val availableDevices: StateFlow<List<Device>> = _availableDevices

    private val _selectedDevice = MutableStateFlow<Device?>(null)
    val selectedDevice: StateFlow<Device?> = _selectedDevice

    val sensorData: StateFlow<SensorData?> = raspberryPiClient.sensorData
    val detectedTargets: StateFlow<List<DetectedTarget>> = raspberryPiClient.detectedTargets
    val shootingSolution: StateFlow<ShootingSolution?> = raspberryPiClient.shootingSolution
    val systemStatus: StateFlow<SystemStatus> = raspberryPiClient.systemStatus

    fun navigateToDeviceList() {
        _uiState.value = _uiState.value.copy(currentScreen = AppScreen.DEVICE_SELECTION)
    }

    fun navigateToMap() {
        _uiState.value = _uiState.value.copy(currentScreen = AppScreen.MAP)
    }

    fun navigateToHome() {
        _uiState.value = _uiState.value.copy(currentScreen = AppScreen.HOME)
    }

    fun selectDevice(device: Device) {
        Log.d("SniperViewModel", "selectDevice called for: ${device.name}")
        _selectedDevice.value = device
        _uiState.value = _uiState.value.copy(
            currentScreen = AppScreen.DASHBOARD,
            selectedDeviceId = device.id
        )
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

    fun goBackToDeviceSelection() {
        _uiState.value = _uiState.value.copy(currentScreen = AppScreen.DEVICE_SELECTION)
    }

    fun goBackToHome() {
        Log.d("SniperViewModel", "goBackToHome called")
        _uiState.value = _uiState.value.copy(currentScreen = AppScreen.HOME)
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
    val currentScreen: AppScreen = AppScreen.HOME,
    val isScanning: Boolean = false,
    val selectedDeviceId: String? = null,
    val connectionError: String? = null,
    val isVideoFullscreen: Boolean = false
)

enum class AppScreen {
    HOME,
    DEVICE_SELECTION,
    MAP,
    DASHBOARD
}
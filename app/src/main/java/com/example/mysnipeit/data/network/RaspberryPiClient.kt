package com.example.mysnipeit.data.network

import com.example.mysnipeit.data.models.*
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import android.util.Log

class RaspberryPiClient {

    // StateFlow for reactive UI updates
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

    fun connect(ipAddress: String, port: Int = 8765) {
        Log.d("RaspberryPiClient", "Attempting to connect to $ipAddress:$port")

        // Simulate connection process for now
        _systemStatus.value = _systemStatus.value.copy(
            connectionStatus = ConnectionState.CONNECTING
        )

        // TODO: Implement actual WebSocket connection later
        // For now, simulate successful connection after delay
        android.os.Handler(android.os.Looper.getMainLooper()).postDelayed({
            _systemStatus.value = _systemStatus.value.copy(
                connectionStatus = ConnectionState.CONNECTED,
                batteryLevel = 87
            )

            // Simulate some mock data
            simulateMockData()
        }, 2000)
    }

    fun disconnect() {
        Log.d("RaspberryPiClient", "Disconnecting from RPi5")
        _systemStatus.value = _systemStatus.value.copy(
            connectionStatus = ConnectionState.DISCONNECTED
        )
    }

    fun sendCommand(command: String, data: Any? = null) {
        Log.d("RaspberryPiClient", "Sending command: $command")
        // TODO: Implement actual command sending
    }

    private fun simulateMockData() {
        // Simulate sensor data
        _sensorData.value = SensorData(
            temperature = 22.5,
            humidity = 65.0,
            windDirection = 270.0,
            windSpeed = 3.2,
            rangefinderDistance = 450.0,
            gpsLatitude = 35.093,
            gpsLongitude = 32.014,
            timestamp = System.currentTimeMillis()
        )

        // Simulate shooting solution
        _shootingSolution.value = ShootingSolution(
            azimuth = 245.5,
            elevation = 12.3,
            windageAdjustment = 0.2,
            elevationAdjustment = 0.8,
            confidence = 0.85f,
            timestamp = System.currentTimeMillis()
        )
    }
}
package com.example.mysnipeit.data.repository

import com.example.mysnipeit.data.models.*
import com.example.mysnipeit.data.network.RaspberryPiClient
import kotlinx.coroutines.flow.StateFlow

class SniperRepository {

    private val raspberryPiClient = RaspberryPiClient()

    // Expose data streams from the network client
    val sensorData: StateFlow<SensorData?> = raspberryPiClient.sensorData
    val detectedTargets: StateFlow<List<DetectedTarget>> = raspberryPiClient.detectedTargets
    val shootingSolution: StateFlow<ShootingSolution?> = raspberryPiClient.shootingSolution
    val systemStatus: StateFlow<SystemStatus> = raspberryPiClient.systemStatus

    // Connection management
    fun connectToSystem(ipAddress: String, port: Int = 8765) {
        raspberryPiClient.connect(ipAddress, port)
    }

    fun disconnectFromSystem() {
        raspberryPiClient.disconnect()
    }

    // Commands to send to RPi5
    fun selectTarget(targetId: String) {
        raspberryPiClient.sendCommand("select_target", mapOf("target_id" to targetId))
    }

    fun requestCalibration() {
        raspberryPiClient.sendCommand("calibrate_system")
    }

    fun setManualTarget(latitude: Double, longitude: Double) {
        raspberryPiClient.sendCommand("set_manual_target", mapOf(
            "latitude" to latitude,
            "longitude" to longitude
        ))
    }

    fun emergencyStop() {
        raspberryPiClient.sendCommand("emergency_stop")
    }
}
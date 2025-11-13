package com.example.mysnipeit.data.repository

import com.example.mysnipeit.data.models.*
import com.example.mysnipeit.data.network.RaspberryPiClient
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.launch

class SniperRepository {

    private val raspberryPiClient = RaspberryPiClient()
    private val scope = CoroutineScope(Dispatchers.IO + SupervisorJob())

    // Store current IP address for sending commands
    private var currentIpAddress: String? = null

    // Expose data streams from the network client
    val sensorData: StateFlow<SensorData?> = raspberryPiClient.sensorData
    val detectedTargets: StateFlow<List<DetectedTarget>> = raspberryPiClient.detectedTargets
    val shootingSolution: StateFlow<ShootingSolution?> = raspberryPiClient.shootingSolution
    val systemStatus: StateFlow<SystemStatus> = raspberryPiClient.systemStatus


    suspend fun connectToSystem(ipAddress: String) {
        currentIpAddress = ipAddress
        raspberryPiClient.connect(ipAddress)
    }

    fun disconnectFromSystem() {
        raspberryPiClient.disconnect()
        currentIpAddress = null
    }

    // Commands to send to RPi5 - FIXED to use suspend functions
    fun selectTarget(targetId: String) {
        scope.launch {
            currentIpAddress?.let { ip ->
                raspberryPiClient.sendCommand(
                    ipAddress = ip,
                    command = "select_target",
                    params = mapOf("target_id" to targetId)
                )
            }
        }
    }

    fun requestCalibration() {
        scope.launch {
            currentIpAddress?.let { ip ->
                raspberryPiClient.sendCommand(
                    ipAddress = ip,
                    command = "calibrate_system",
                    params = emptyMap()
                )
            }
        }
    }

    fun setManualTarget(latitude: Double, longitude: Double) {
        scope.launch {
            currentIpAddress?.let { ip ->
                raspberryPiClient.sendCommand(
                    ipAddress = ip,
                    command = "set_manual_target",
                    params = mapOf(
                        "latitude" to latitude,
                        "longitude" to longitude
                    )
                )
            }
        }
    }

    fun emergencyStop() {
        scope.launch {
            currentIpAddress?.let { ip ->
                raspberryPiClient.sendCommand(
                    ipAddress = ip,
                    command = "emergency_stop",
                    params = emptyMap()
                )
            }
        }
    }

    // Additional helper methods

    fun getVideoStreamUrl(): String? {
        return currentIpAddress?.let { ip ->
            raspberryPiClient.getVideoStreamUrl(ip)
        }
    }

    fun isConnected(): Boolean {
        return raspberryPiClient.isConnected()
    }
}
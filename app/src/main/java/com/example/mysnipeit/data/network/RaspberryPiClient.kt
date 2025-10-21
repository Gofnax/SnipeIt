package com.example.mysnipeit.data.network

import com.example.mysnipeit.data.models.*
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import android.util.Log
import kotlinx.coroutines.*
import kotlin.random.Random

class RaspberryPiClient {

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

    private var simulationJob: Job? = null
    private var isConnected = false

    // Mock data parameters
    private var mockTemperature = 22.5
    private var mockWindDirection = 270.0
    private var mockWindSpeed = 3.2

    fun connect(ipAddress: String, port: Int = 8765) {
        Log.d("RaspberryPiClient", "Attempting to connect to $ipAddress:$port")

        _systemStatus.value = _systemStatus.value.copy(
            connectionStatus = ConnectionState.CONNECTING
        )

        // Simulate connection process
        android.os.Handler(android.os.Looper.getMainLooper()).postDelayed({
            isConnected = true

            _systemStatus.value = _systemStatus.value.copy(
                connectionStatus = ConnectionState.CONNECTED,
                batteryLevel = Random.nextInt(60, 100),
                cameraStatus = true,
                gpsStatus = true,
                rangefinderStatus = true,
                microphoneStatus = true,
                lastHeartbeat = System.currentTimeMillis()
            )

            Log.d("RaspberryPiClient", "Connected! Starting data simulation...")
            startDataSimulation()
        }, 2000)
    }

    fun disconnect() {
        Log.d("RaspberryPiClient", "Disconnecting from RPi5")
        isConnected = false
        simulationJob?.cancel()

        _systemStatus.value = _systemStatus.value.copy(
            connectionStatus = ConnectionState.DISCONNECTED,
            batteryLevel = null,
            cameraStatus = false,
            gpsStatus = false,
            rangefinderStatus = false,
            microphoneStatus = false
        )

        // Clear data
        _sensorData.value = null
        _shootingSolution.value = null
        _detectedTargets.value = emptyList()
    }

    fun sendCommand(command: String, data: Any? = null) {
        Log.d("RaspberryPiClient", "Sending command: $command")
    }

    private fun startDataSimulation() {
        simulationJob?.cancel() // Cancel any existing job

        simulationJob = CoroutineScope(Dispatchers.Main).launch {
            Log.d("RaspberryPiClient", "Data simulation started!")

            while (isConnected) {
                try {
                    // Update sensor data
                    updateMockSensorData()

                    // Update targets occasionally
                    if (Random.nextFloat() < 0.3f) {
                        updateMockTargets()
                    }

                    // Update shooting solution
                    updateShootingSolution()

                    // Update battery occasionally
                    if (Random.nextFloat() < 0.1f) {
                        updateBatteryLevel()
                    }

                    delay(500) // Update every 500ms
                } catch (e: Exception) {
                    Log.e("RaspberryPiClient", "Simulation error: ${e.message}")
                }
            }
        }
    }

    private fun updateMockSensorData() {
        // Simulate realistic sensor variations
        mockTemperature += Random.nextDouble(-0.2, 0.2)
        mockTemperature = mockTemperature.coerceIn(15.0, 35.0)

        mockWindDirection += Random.nextDouble(-5.0, 5.0)
        mockWindDirection = (mockWindDirection + 360) % 360

        mockWindSpeed += Random.nextDouble(-0.3, 0.3)
        mockWindSpeed = mockWindSpeed.coerceIn(0.0, 15.0)

        val newSensorData = SensorData(
            temperature = mockTemperature,
            humidity = Random.nextDouble(50.0, 80.0),
            windDirection = mockWindDirection,
            windSpeed = mockWindSpeed,
            rangefinderDistance = 450.0 + Random.nextDouble(-10.0, 10.0),
            gpsLatitude = 35.093 + Random.nextDouble(-0.001, 0.001),
            gpsLongitude = 32.014 + Random.nextDouble(-0.001, 0.001),
            timestamp = System.currentTimeMillis()
        )

        _sensorData.value = newSensorData
        Log.d("RaspberryPiClient", "Sensor data updated: Temp=${mockTemperature.toInt()}°C, Wind=${mockWindSpeed.toInt()}m/s")
    }

    private fun updateMockTargets() {
        val targetScenarios = listOf(
            emptyList(),
            listOf(createMockTarget("H1", TargetType.HUMAN, 420.0, 245.5f, 0.85f)),
            listOf(
                createMockTarget("H1", TargetType.HUMAN, 380.0, 240.0f, 0.92f),
                createMockTarget("V1", TargetType.VEHICLE, 650.0, 260.0f, 0.78f)
            )
        )

        _detectedTargets.value = targetScenarios.random()
    }

    private fun createMockTarget(
        id: String,
        type: TargetType,
        distance: Double,
        bearing: Float,
        confidence: Float
    ): DetectedTarget {
        return DetectedTarget(
            id = id,
            confidence = confidence,
            screenX = Random.nextFloat(),
            screenY = Random.nextFloat(),
            worldLatitude = 35.093 + Random.nextDouble(-0.01, 0.01),
            worldLongitude = 32.014 + Random.nextDouble(-0.01, 0.01),
            distance = distance,
            bearing = bearing.toDouble(),
            targetType = type,
            timestamp = System.currentTimeMillis()
        )
    }

    private fun updateShootingSolution() {
        val targets = _detectedTargets.value
        if (targets.isNotEmpty()) {
            val bestTarget = targets.first()

            _shootingSolution.value = ShootingSolution(
                azimuth = bestTarget.bearing + Random.nextDouble(-1.0, 1.0),
                elevation = calculateMockElevation(bestTarget.distance),
                windageAdjustment = Random.nextDouble(-0.5, 0.5),
                elevationAdjustment = Random.nextDouble(-0.5, 0.5),
                confidence = bestTarget.confidence,
                timestamp = System.currentTimeMillis()
            )
        } else {
            _shootingSolution.value = null
        }
    }

    private fun calculateMockElevation(distance: Double): Double {
        return when {
            distance < 300 -> Random.nextDouble(8.0, 12.0)
            distance < 600 -> Random.nextDouble(12.0, 18.0)
            else -> Random.nextDouble(18.0, 25.0)
        }
    }

    private fun updateBatteryLevel() {
        val currentBattery = _systemStatus.value.batteryLevel ?: 100
        val newBattery = (currentBattery - Random.nextInt(0, 2)).coerceIn(20, 100)

        _systemStatus.value = _systemStatus.value.copy(
            batteryLevel = newBattery,
            lastHeartbeat = System.currentTimeMillis()
        )
    }
}
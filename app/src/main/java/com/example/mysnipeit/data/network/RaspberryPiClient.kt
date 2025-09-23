package com.example.mysnipeit.data.network

import com.example.mysnipeit.data.models.*
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import android.util.Log
import kotlinx.coroutines.*
import kotlin.random.Random

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

    private var simulationJob: Job? = null
    private var isConnected = false

    // Mock data parameters
    private var mockTargetCount = 0
    private var mockDistance = 450.0
    private var mockAzimuth = 245.0
    private var mockElevation = 12.0
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
            _systemStatus.value = _systemStatus.value.copy(
                connectionStatus = ConnectionState.CONNECTED,
                batteryLevel = Random.nextInt(60, 100),
                cameraStatus = true,
                gpsStatus = true,
                rangefinderStatus = true,
                microphoneStatus = true,
                lastHeartbeat = System.currentTimeMillis()
            )

            isConnected = true
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

        when (command) {
            "select_target" -> {
                // Simulate target lock-on
                simulateTargetLockOn()
            }
            "calibrate_system" -> {
                // Simulate calibration
                simulateCalibration()
            }
            "emergency_stop" -> {
                // Clear all targets and reset
                _detectedTargets.value = emptyList()
                _shootingSolution.value = null
            }
        }
    }

    private fun startDataSimulation() {
        simulationJob = CoroutineScope(Dispatchers.Main).launch {
            while (isConnected) {
                // Update sensor data every 500ms
                updateMockSensorData()

                // Update targets every 2-5 seconds
                if (Random.nextFloat() < 0.3f) { // 30% chance each cycle
                    updateMockTargets()
                }

                // Update shooting solution based on targets
                updateShootingSolution()

                // Update battery level occasionally
                if (Random.nextFloat() < 0.1f) { // 10% chance
                    updateBatteryLevel()
                }

                delay(500) // Update every 500ms
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

        _sensorData.value = SensorData(
            temperature = mockTemperature,
            humidity = Random.nextDouble(50.0, 80.0),
            windDirection = mockWindDirection,
            windSpeed = mockWindSpeed,
            rangefinderDistance = mockDistance + Random.nextDouble(-10.0, 10.0),
            gpsLatitude = 35.093 + Random.nextDouble(-0.001, 0.001),
            gpsLongitude = 32.014 + Random.nextDouble(-0.001, 0.001),
            timestamp = System.currentTimeMillis()
        )
    }

    private fun updateMockTargets() {
        val targetScenarios = listOf(
            // Scenario 1: No targets
            emptyList(),

            // Scenario 2: Single human target
            listOf(
                createMockTarget("H1", TargetType.HUMAN, 420.0, 245.5f, 0.85f)
            ),

            // Scenario 3: Multiple targets
            listOf(
                createMockTarget("H1", TargetType.HUMAN, 380.0, 240.0f, 0.92f),
                createMockTarget("V1", TargetType.VEHICLE, 650.0, 260.0f, 0.78f),
                createMockTarget("H2", TargetType.HUMAN, 290.0, 235.0f, 0.65f)
            ),

            // Scenario 4: Distant targets
            listOf(
                createMockTarget("S1", TargetType.STRUCTURE, 800.0, 270.0f, 0.45f),
                createMockTarget("U1", TargetType.UNKNOWN, 920.0, 280.0f, 0.35f)
            )
        )

        // Select a random scenario
        val selectedTargets = targetScenarios.random()
        _detectedTargets.value = selectedTargets

        Log.d("MockData", "Updated targets: ${selectedTargets.size} detected")
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
            screenX = Random.nextFloat() * 1920, // Simulate screen coordinates
            screenY = Random.nextFloat() * 1080,
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
            // Calculate solution for the closest high-confidence target
            val bestTarget = targets
                .filter { it.confidence > 0.6f }
                .minByOrNull { it.distance }

            if (bestTarget != null) {
                // Simulate ballistic calculations with some variation
                mockAzimuth = bestTarget.bearing + Random.nextDouble(-1.0, 1.0)
                mockElevation = calculateMockElevation(bestTarget.distance)

                _shootingSolution.value = ShootingSolution(
                    azimuth = mockAzimuth,
                    elevation = mockElevation,
                    windageAdjustment = calculateWindageAdjustment(),
                    elevationAdjustment = Random.nextDouble(-0.5, 0.5),
                    confidence = bestTarget.confidence * Random.nextFloat() * 0.2f + 0.8f, // 80-100% confidence
                    timestamp = System.currentTimeMillis()
                )

                Log.d("MockData", "Shooting solution: Az=${mockAzimuth.format(1)}°, El=${mockElevation.format(1)}°")
            }
        } else {
            _shootingSolution.value = null
        }
    }

    private fun calculateMockElevation(distance: Double): Double {
        // Simulate elevation based on distance (simplified ballistic curve)
        return when {
            distance < 300 -> Random.nextDouble(8.0, 12.0)
            distance < 600 -> Random.nextDouble(12.0, 18.0)
            distance < 900 -> Random.nextDouble(18.0, 25.0)
            else -> Random.nextDouble(25.0, 35.0)
        }
    }

    private fun calculateWindageAdjustment(): Double {
        val sensorData = _sensorData.value ?: return 0.0

        // Simple windage calculation based on wind speed and direction
        val windEffect = sensorData.windSpeed * 0.3 // Simplified calculation
        val windAngle = Math.toRadians(sensorData.windDirection)

        return windEffect * kotlin.math.sin(windAngle)
    }

    private fun updateBatteryLevel() {
        val currentBattery = _systemStatus.value.batteryLevel ?: 100
        val newBattery = (currentBattery - Random.nextInt(0, 2)).coerceIn(20, 100)

        _systemStatus.value = _systemStatus.value.copy(
            batteryLevel = newBattery,
            lastHeartbeat = System.currentTimeMillis()
        )
    }

    private fun simulateTargetLockOn() {
        // Simulate target lock-on with enhanced precision
        val targets = _detectedTargets.value
        if (targets.isNotEmpty()) {
            val lockedTarget = targets.first()

            _shootingSolution.value = ShootingSolution(
                azimuth = lockedTarget.bearing,
                elevation = calculateMockElevation(lockedTarget.distance),
                windageAdjustment = calculateWindageAdjustment(),
                elevationAdjustment = 0.0,
                confidence = 0.95f, // High confidence when locked
                timestamp = System.currentTimeMillis()
            )

            Log.d("MockData", "Target locked: ${lockedTarget.targetType} at ${lockedTarget.distance}m")
        }
    }

    private fun simulateCalibration() {
        Log.d("MockData", "Starting system calibration...")

        // Simulate calibration process
        CoroutineScope(Dispatchers.Main).launch {
            _systemStatus.value = _systemStatus.value.copy(
                cameraStatus = false,
                gpsStatus = false
            )

            delay(3000) // Calibration takes 3 seconds

            _systemStatus.value = _systemStatus.value.copy(
                cameraStatus = true,
                gpsStatus = true
            )

            Log.d("MockData", "Calibration complete")
        }
    }

    // Extension function for number formatting
    private fun Double.format(digits: Int) = "%.${digits}f".format(this)
}
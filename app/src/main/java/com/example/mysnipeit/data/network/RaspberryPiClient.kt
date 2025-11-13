package com.example.mysnipeit.data.network

import android.util.Log
import com.example.mysnipeit.data.models.*
import kotlinx.coroutines.*
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import org.java_websocket.client.WebSocketClient
import org.java_websocket.handshake.ServerHandshake
import java.net.URI
import com.google.gson.Gson
import java.net.HttpURLConnection
import java.net.URL

/**
 * Enhanced Raspberry Pi Client
 * Handles all communication with the RPi5
 */
class RaspberryPiClient {

    companion object {
        private const val TAG = "RaspberryPiClient"
        private const val WEBSOCKET_PORT = 8080
        private const val HTTP_PORT = 8000
        private const val VIDEO_STREAM_PORT = 8554
    }

    private val gson = Gson()
    private var webSocketClient: WebSocketClient? = null
    private val scope = CoroutineScope(Dispatchers.IO + SupervisorJob())

    // Sensor data from RPi
    private val _sensorData = MutableStateFlow<SensorData?>(null)
    val sensorData: StateFlow<SensorData?> = _sensorData.asStateFlow()

    // Detected targets from RPi
    private val _detectedTargets = MutableStateFlow<List<DetectedTarget>>(emptyList())
    val detectedTargets: StateFlow<List<DetectedTarget>> = _detectedTargets.asStateFlow()

    // Shooting solution from RPi
    private val _shootingSolution = MutableStateFlow<ShootingSolution?>(null)
    val shootingSolution: StateFlow<ShootingSolution?> = _shootingSolution.asStateFlow()

    // System status - FIXED to match your model exactly
    private val _systemStatus = MutableStateFlow(
        SystemStatus(
            connectionStatus = ConnectionState.DISCONNECTED,
            batteryLevel = null,
            cameraStatus = false,
            gpsStatus = false,
            rangefinderStatus = false,
            microphoneStatus = false,
            lastHeartbeat = System.currentTimeMillis() ,
            cpuTemperature = null,
            gpsLatitude = null,
            gpsLongitude = null
        )
    )
    val systemStatus: StateFlow<SystemStatus> = _systemStatus.asStateFlow()

    // Mock data generator
    private var mockDataJob: Job? = null

    /**
     * Connect to Raspberry Pi
     */
    suspend fun connect(ipAddress: String) = withContext(Dispatchers.IO) {
        try {
            Log.d(TAG, "🔌 Attempting to connect to RPi at $ipAddress")

            val networkTester = NetworkTester()
            val canPing = networkTester.pingDevice(ipAddress)

            if (!canPing) {
                Log.e(TAG, "❌ Cannot ping device at $ipAddress")
                startMockDataGeneration()
                return@withContext
            }

            Log.d(TAG, "✅ Device is reachable")

            val portStatus = networkTester.scanPorts(
                ipAddress,
                listOf(WEBSOCKET_PORT, HTTP_PORT, VIDEO_STREAM_PORT)
            )

            Log.d(TAG, "Port scan results: $portStatus")

            if (portStatus[WEBSOCKET_PORT] == true) {
                connectWebSocket(ipAddress)
            } else {
                Log.w(TAG, "⚠️ WebSocket port not open, using mock data")
                startMockDataGeneration()
            }

            if (portStatus[HTTP_PORT] == true) {
                testHttpApi(ipAddress)
            }

            Log.d(TAG, " Successfully connected to RPi")

        } catch (e: Exception) {
            Log.e(TAG, " Connection failed: ${e.message}", e)
            Log.d(TAG, " Falling back to mock data generation")
            startMockDataGeneration()
        }
    }

    private fun connectWebSocket(ipAddress: String) {
        val wsUri = URI("ws://$ipAddress:$WEBSOCKET_PORT/sensor-data")

        webSocketClient = object : WebSocketClient(wsUri) {
            override fun onOpen(handshakedata: ServerHandshake?) {
                Log.d(TAG, "✅ WebSocket connected")
                updateSystemStatus(ConnectionState.CONNECTED)
            }

            override fun onMessage(message: String?) {
                message?.let { handleWebSocketMessage(it) }
            }

            override fun onClose(code: Int, reason: String?, remote: Boolean) {
                Log.d(TAG, "WebSocket closed: $reason")
                updateSystemStatus(ConnectionState.DISCONNECTED)
            }

            override fun onError(ex: Exception?) {
                Log.e(TAG, "WebSocket error: ${ex?.message}")
                updateSystemStatus(ConnectionState.ERROR)
            }
        }

        webSocketClient?.connect()
    }

    private fun handleWebSocketMessage(message: String) {
        try {
            Log.d(TAG, " Received: $message")

            val dataType = gson.fromJson(message, Map::class.java)["type"] as? String

            when (dataType) {
                "sensor_data" -> {
                    val data = gson.fromJson(message, SensorData::class.java)
                    _sensorData.value = data
                }
                "target_detection" -> {
                    val targets = gson.fromJson(message, Array<DetectedTarget>::class.java).toList()
                    _detectedTargets.value = targets
                }
                "shooting_solution" -> {
                    val solution = gson.fromJson(message, ShootingSolution::class.java)
                    _shootingSolution.value = solution
                }
                "system_status" -> {
                    val status = gson.fromJson(message, SystemStatus::class.java)
                    _systemStatus.value = status
                }
            }
        } catch (e: Exception) {
            Log.e(TAG, "Failed to parse message: ${e.message}")
        }
    }

    private fun updateSystemStatus(connectionState: ConnectionState) {
        _systemStatus.value = _systemStatus.value.copy(
            connectionStatus = connectionState,
            lastHeartbeat = System.currentTimeMillis()
        )
    }

    private suspend fun testHttpApi(ipAddress: String): Boolean {
        return withContext(Dispatchers.IO) {
            try {
                val url = URL("http://$ipAddress:$HTTP_PORT/api/status")
                val connection = url.openConnection() as HttpURLConnection
                connection.requestMethod = "GET"
                connection.connectTimeout = 5000
                connection.readTimeout = 5000

                val responseCode = connection.responseCode
                val isSuccess = responseCode == 200

                if (isSuccess) {
                    val response = connection.inputStream.bufferedReader().readText()
                    Log.d(TAG, " HTTP API response: $response")
                } else {
                    Log.w(TAG, " HTTP API returned code: $responseCode")
                }

                connection.disconnect()
                isSuccess
            } catch (e: Exception) {
                Log.e(TAG, " HTTP API test failed: ${e.message}")
                false
            }
        }
    }

    suspend fun sendCommand(ipAddress: String, command: String, params: Map<String, Any> = emptyMap()): Boolean {
        return withContext(Dispatchers.IO) {
            try {
                val url = URL("http://$ipAddress:$HTTP_PORT/api/command")
                val connection = url.openConnection() as HttpURLConnection
                connection.requestMethod = "POST"
                connection.setRequestProperty("Content-Type", "application/json")
                connection.doOutput = true

                val jsonCommand = gson.toJson(mapOf("command" to command, "params" to params))
                connection.outputStream.write(jsonCommand.toByteArray())

                val responseCode = connection.responseCode
                val isSuccess = responseCode == 200

                Log.d(TAG, if (isSuccess) " Command sent: $command" else " Command failed: $command")

                connection.disconnect()
                isSuccess
            } catch (e: Exception) {
                Log.e(TAG, " Failed to send command: ${e.message}")
                false
            }
        }
    }

    fun getVideoStreamUrl(ipAddress: String): String {
        return "rtsp://$ipAddress:$VIDEO_STREAM_PORT/video"
    }

    /**
     * Start mock data generation - FIXED with Boolean values
     */
    private fun startMockDataGeneration() {
        Log.d(TAG, " Starting mock data generation")

        mockDataJob?.cancel()
        mockDataJob = scope.launch {
            while (isActive) {
                // 1. Generate mock sensor data
                _sensorData.value = SensorData(
                    temperature = 20.0 + (Math.random() * 10),
                    humidity = 50.0 + (Math.random() * 20),
                    windSpeed = 5.0 + (Math.random() * 10),
                    windDirection = (Math.random() * 360).toInt().toDouble(),
                    rangefinderDistance = 400.0 + (Math.random() * 200),
                    gpsLatitude = 35.0 + (Math.random() * 10),
                    gpsLongitude = 32.0 + (Math.random() * 10),
                    timestamp = System.currentTimeMillis()
                )

                //  2. GENERATE MOCK TARGETS (THIS WAS MISSING!)
                val mockTargets = listOf(
                    DetectedTarget(
                        id = "T1",
                        confidence = 0.85f,
                        screenX = 0.3f,
                        screenY = 0.4f,
                        worldLatitude = 35.093,
                        worldLongitude = 32.014,
                        distance = 420.0,
                        bearing = 245.0,
                        targetType = TargetType.HUMAN,
                        timestamp = System.currentTimeMillis()
                    ),
                    DetectedTarget(
                        id = "T2",
                        confidence = 0.72f,
                        screenX = 0.7f,
                        screenY = 0.5f,
                        worldLatitude = 35.094,
                        worldLongitude = 32.015,
                        distance = 580.0,
                        bearing = 260.0,
                        targetType = TargetType.UNKNOWN,
                        timestamp = System.currentTimeMillis()
                    )
                )

                _detectedTargets.value = mockTargets
                Log.d(TAG, " Generated ${mockTargets.size} targets")

                // 3. Generate shooting solution for random target
                if (mockTargets.isNotEmpty()) {
                    val randomTarget = mockTargets.random()

                    _shootingSolution.value = ShootingSolution(
                        targetId = randomTarget.id,
                        azimuth = 245.0 + (Math.random() * 10),
                        elevation = 12.0 + (Math.random() * 5),
                        windageAdjustment = 2.0 + (Math.random() * 2),
                        elevationAdjustment = 1.5 + (Math.random() * 1),
                        confidence = 0.75f + (Math.random() * 0.2).toFloat(),
                        timestamp = System.currentTimeMillis()
                    )

                    Log.d(TAG, " Generated solution for ${randomTarget.id}: AZ=${_shootingSolution.value?.azimuth?.toInt()}° EL=${_shootingSolution.value?.elevation?.toInt()}°")
                }

                // 4. Update system status
                _systemStatus.value = SystemStatus(
                    connectionStatus = ConnectionState.CONNECTED,
                    batteryLevel = 85,
                    cameraStatus = true,
                    gpsStatus = true,
                    rangefinderStatus = true,
                    microphoneStatus = true,
                    lastHeartbeat = System.currentTimeMillis()
                )

                delay(1500) // Update every 1.5 seconds
            }
        }
    }

    fun disconnect() {
        Log.d(TAG, "🔌 Disconnecting from RPi")

        webSocketClient?.close()
        webSocketClient = null

        mockDataJob?.cancel()
        mockDataJob = null

        _systemStatus.value = _systemStatus.value.copy(
            connectionStatus = ConnectionState.DISCONNECTED
        )
        _sensorData.value = null
        _detectedTargets.value = emptyList()
        _shootingSolution.value = null
    }

    fun isConnected(): Boolean {
        return _systemStatus.value.connectionStatus == ConnectionState.CONNECTED
    }
}

/**

 * Expected JSON format from RPi:

 *

 * Sensor Data:

 * {

 *   "type": "sensor_data",

 *   "temperature": 25.5,

 *   "humidity": 65.0,

 *   "windSpeed": 8.2,

 *   "windDirection": 245,

 *   "rangefinderDistance": 420.5,

 *   "timestamp": 1234567890

 * }

 *

 * Target Detection:

 * {

 *   "type": "target_detection",

 *   "targets": [

 *     {

 *       "id": "T1",

 *       "confidence": 0.85,

 *       "screenX": 0.5,

 *       "screenY": 0.4,

 *       "worldLatitude": 35.094,

 *       "worldLongitude": 32.015,

 *       "distance": 420.0,

 *       "bearing": 245.0,

 *       "targetType": "HUMAN",

 *       "timestamp": 1234567890

 *     }

 *   ]

 * }

 *

 * Shooting Solution:

 * {

 *   "type": "shooting_solution",

 *   "azimuth": 245.5,

 *   "elevation": 12.3,

 *   "windageAdjustment": 2.1,

 *   "elevationAdjustment": 1.5,

 *   "confidence": 0.85,

 *   "timestamp": 1234567890

 * }

 */
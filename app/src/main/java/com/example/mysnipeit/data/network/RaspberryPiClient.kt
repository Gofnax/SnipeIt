package com.example.mysnipeit.data.network


import com.example.mysnipeit.data.models.*
import com.google.gson.Gson
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import org.java_websocket.client.WebSocketClient
import org.java_websocket.handshake.ServerHandshake
import java.net.URI
import android.util.Log

class RaspberryPiClient {
    private var webSocketClient: WebSocketClient? = null
    private val gson = Gson()

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
        try {
            val uri = URI("ws://$ipAddress:$port")

            webSocketClient = object : WebSocketClient(uri) {
                override fun onOpen(handshake: ServerHandshake?) {
                    Log.d("WebSocket", "Connected to RPi5")
                    _systemStatus.value = _systemStatus.value.copy(
                        connectionStatus = ConnectionState.CONNECTED
                    )
                }

                override fun onMessage(message: String?) {
                    message?.let { parseMessage(it) }
                }

                override fun onClose(code: Int, reason: String?, remote: Boolean) {
                    Log.d("WebSocket", "Connection closed: $reason")
                    _systemStatus.value = _systemStatus.value.copy(
                        connectionStatus = ConnectionState.DISCONNECTED
                    )
                }

                override fun onError(ex: Exception?) {
                    Log.e("WebSocket", "Error: ${ex?.message}")
                    _systemStatus.value = _systemStatus.value.copy(
                        connectionStatus = ConnectionState.ERROR
                    )
                }
            }

            _systemStatus.value = _systemStatus.value.copy(
                connectionStatus = ConnectionState.CONNECTING
            )
            webSocketClient?.connect()

        } catch (e: Exception) {
            Log.e("RaspberryPiClient", "Connection failed: ${e.message}")
            _systemStatus.value = _systemStatus.value.copy(
                connectionStatus = ConnectionState.ERROR
            )
        }
    }

    private fun parseMessage(message: String) {
        try {
            val messageData = gson.fromJson(message, MessageWrapper::class.java)

            when (messageData.type) {
                "sensor_data" -> {
                    val sensorData = gson.fromJson(messageData.payload, SensorData::class.java)
                    _sensorData.value = sensorData
                }
                "targets" -> {
                    val targets = gson.fromJson(messageData.payload, Array<DetectedTarget>::class.java)
                    _detectedTargets.value = targets.toList()
                }
                "shooting_solution" -> {
                    val solution = gson.fromJson(messageData.payload, ShootingSolution::class.java)
                    _shootingSolution.value = solution
                }
                "system_status" -> {
                    val status = gson.fromJson(messageData.payload, SystemStatus::class.java)
                    _systemStatus.value = status.copy(lastHeartbeat = System.currentTimeMillis())
                }
            }
        } catch (e: Exception) {
            Log.e("RaspberryPiClient", "Failed to parse message: ${e.message}")
        }
    }

    fun disconnect() {
        webSocketClient?.close()
        webSocketClient = null
    }

    fun sendCommand(command: String, data: Any? = null) {
        val message = MessageWrapper(command, gson.toJson(data))
        webSocketClient?.send(gson.toJson(message))
    }

    data class MessageWrapper(
        val type: String,
        val payload: String
    )
}
package com.example.mysnipeit.data.models

data class SystemStatus(
    val connectionStatus: ConnectionState,
    val batteryLevel: Int?,
    val cameraStatus: Boolean,
    val gpsStatus: Boolean,
    val rangefinderStatus: Boolean,
    val microphoneStatus: Boolean,
    val lastHeartbeat: Long
)

enum class ConnectionState {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    ERROR
}
package com.example.mysnipeit.data.models

import android.os.Parcelable
import kotlinx.parcelize.Parcelize

@Parcelize
data class SensorData(
    val temperature: Double,
    val humidity: Double,
    val windDirection: Double, // degrees
    val windSpeed: Double, // m/s
    val rangefinderDistance: Double, // meters
    val gpsLatitude: Double,
    val gpsLongitude: Double,
    val timestamp: Long
) : Parcelable

@Parcelize
data class DetectedTarget(
    val id: String,
    val confidence: Float,
    val screenX: Float, // Position in video feed
    val screenY: Float,
    val worldLatitude: Double, // Calculated real-world position
    val worldLongitude: Double,
    val distance: Double,
    val bearing: Double, // degrees from north
    val targetType: TargetType,
    val timestamp: Long
) : Parcelable

@Parcelize
data class ShootingSolution(
    val azimuth: Double, // degrees
    val elevation: Double, // degrees
    val windageAdjustment: Double,
    val elevationAdjustment: Double,
    val confidence: Float,
    val timestamp: Long
) : Parcelable

@Parcelize
data class SystemStatus(
    val connectionStatus: ConnectionState,
    val batteryLevel: Int?,
    val cameraStatus: Boolean,
    val gpsStatus: Boolean,
    val rangefinderStatus: Boolean,
    val microphoneStatus: Boolean,
    val lastHeartbeat: Long
) : Parcelable

enum class TargetType {
    HUMAN,
    VEHICLE,
    STRUCTURE,
    UNKNOWN
}

enum class ConnectionState {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    ERROR
}
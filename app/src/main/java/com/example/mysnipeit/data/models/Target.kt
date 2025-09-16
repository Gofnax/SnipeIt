package com.example.mysnipeit.data.models


data class DetectedTarget(
    val id: String,
    val confidence: Float,
    val screenX: Float,
    val screenY: Float,
    val worldLatitude: Double,
    val worldLongitude: Double,
    val distance: Double,
    val bearing: Double,
    val targetType: TargetType,
    val timestamp: Long
)

enum class TargetType {
    HUMAN,
    VEHICLE,
    STRUCTURE,
    UNKNOWN
}
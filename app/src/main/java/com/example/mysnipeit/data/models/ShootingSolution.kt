package com.example.mysnipeit.data.models

data class ShootingSolution(
    val azimuth: Double,
    val elevation: Double,
    val windageAdjustment: Double,
    val elevationAdjustment: Double,
    val confidence: Float,
    val timestamp: Long
)
package com.example.mysnipeit.data.models

data class SensorData(
    val temperature: Double,
    val humidity: Double,
    val windDirection: Double,
    val windSpeed: Double,
    val rangefinderDistance: Double,
    val gpsLatitude: Double,
    val gpsLongitude: Double,
    val timestamp: Long
)
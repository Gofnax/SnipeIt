package com.example.mysnipeit.ui.dashboard

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Menu
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.compose.ui.graphics.Color
import com.example.mysnipeit.data.models.SensorData
import com.example.mysnipeit.data.models.DetectedTarget
import com.example.mysnipeit.data.models.ShootingSolution
import com.example.mysnipeit.data.models.SystemStatus
import com.example.mysnipeit.data.models.ConnectionState
import com.example.mysnipeit.ui.theme.*
import android.util.Log

@Composable
fun DashboardScreen(
    sensorData: SensorData?,
    detectedTargets: List<DetectedTarget>,
    shootingSolution: ShootingSolution?,
    systemStatus: SystemStatus,
    onConnectClick: () -> Unit,
    onDisconnectClick: () -> Unit,
    onBackClick: () -> Unit = {},
    onMenuClick: () -> Unit = {}
) {
    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(MilitaryDarkBackground)
    ) {
        // Top HUD Bar - Compact data display
        TopHudBar(
            sensorData = sensorData,
            shootingSolution = shootingSolution,
            systemStatus = systemStatus,
            onBackClick = onBackClick,
            onMenuClick = onMenuClick,
            onConnectClick = onConnectClick,
            onDisconnectClick = onDisconnectClick
        )

        // Main Video Feed - 80% of screen
        Box(
            modifier = Modifier
                .fillMaxWidth()
                .weight(1f) // Takes remaining space
        ) {
            VideoFeedSection(
                detectedTargets = detectedTargets,
                modifier = Modifier.fillMaxSize()
            )
        }
    }
}

@Composable
private fun TopHudBar(
    sensorData: SensorData?,
    shootingSolution: ShootingSolution?,
    systemStatus: SystemStatus,
    onBackClick: () -> Unit,
    onMenuClick: () -> Unit,
    onConnectClick: () -> Unit,
    onDisconnectClick: () -> Unit
) {
    Surface(
        modifier = Modifier
            .fillMaxWidth()
            .height(80.dp),
        color = MilitaryCardBackground.copy(alpha = 0.95f)
    ) {
        Row(
            modifier = Modifier
                .fillMaxSize()
                .padding(horizontal = 8.dp, vertical = 6.dp),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.CenterVertically
        ) {
            // Left section - Navigation
            Row(
                verticalAlignment = Alignment.CenterVertically,
                horizontalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                // Back button
                IconButton(
                    onClick = onBackClick,
                    modifier = Modifier
                        .size(32.dp)
                        .background(
                            MilitaryBorderColor,
                            RoundedCornerShape(4.dp)
                        )
                ) {
                    Text(
                        text = "←",
                        color = MilitaryTextPrimary,
                        fontSize = 16.sp,
                        fontWeight = FontWeight.Bold
                    )
                }

                // Menu button
                IconButton(
                    onClick = onMenuClick,
                    modifier = Modifier
                        .size(32.dp)
                        .background(
                            MilitaryBorderColor,
                            RoundedCornerShape(4.dp)
                        )
                ) {
                    Icon(
                        imageVector = Icons.Default.Menu,
                        contentDescription = "Menu",
                        tint = MilitaryTextPrimary,
                        modifier = Modifier.size(16.dp)
                    )
                }
            }

            // Center section - Shooting data
            Row(
                horizontalArrangement = Arrangement.spacedBy(10.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                if (shootingSolution != null) {
                    HudDataBox(
                        label = "AZ",
                        value = "${shootingSolution.azimuth.toInt()}°",
                        isActive = true
                    )
                    HudDataBox(
                        label = "EL",
                        value = "${shootingSolution.elevation.toInt()}°",
                        isActive = true
                    )
                    HudDataBox(
                        label = "CONF",
                        value = "${(shootingSolution.confidence * 100).toInt()}%",
                        isActive = shootingSolution.confidence > 0.7f
                    )
                } else {
                    HudDataBox(label = "AZ", value = "--", isActive = false)
                    HudDataBox(label = "EL", value = "--", isActive = false)
                    HudDataBox(label = "CONF", value = "--", isActive = false)
                }
            }

            // Right section - Environmental data
            Row(
                horizontalArrangement = Arrangement.spacedBy(8.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                // Environmental data
                if (sensorData != null) {
                    HudDataBox(
                        label = "TEMP",
                        value = "${sensorData.temperature.toInt()}°C",
                        isActive = true
                    )
                    HudDataBox(
                        label = "HUM",
                        value = "${sensorData.humidity.toInt()}%",
                        isActive = true
                    )
                    HudDataBox(
                        label = "W.SPD",
                        value = "${sensorData.windSpeed.toInt()}m/s",
                        isActive = true
                    )
                    HudDataBox(
                        label = "W.DIR",
                        value = "${sensorData.windDirection.toInt()}°",
                        isActive = true
                    )
                    HudDataBox(
                        label = "RNG",
                        value = "${sensorData.rangefinderDistance.toInt()}m",
                        isActive = true
                    )
                } else {
                    HudDataBox(label = "TEMP", value = "--", isActive = false)
                    HudDataBox(label = "HUM", value = "--", isActive = false)
                    HudDataBox(label = "W.SPD", value = "--", isActive = false)
                    HudDataBox(label = "W.DIR", value = "--", isActive = false)
                    HudDataBox(label = "RNG", value = "--", isActive = false)
                }
            }

            // Far right section - Status indicators
            Row(
                horizontalArrangement = Arrangement.spacedBy(8.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                // Connection status
                ConnectionStatusCompact(systemStatus)

                // Battery level
                systemStatus.batteryLevel?.let { battery ->
                    HudDataBox(
                        label = "BAT",
                        value = "$battery%",
                        isActive = battery > 20
                    )
                }
            }
        }
    }
}

@Composable
private fun HudDataBox(
    label: String,
    value: String,
    isActive: Boolean
) {
    Column(
        horizontalAlignment = Alignment.CenterHorizontally,
        modifier = Modifier
            .background(
                if (isActive) MilitaryAccentGreen.copy(alpha = 0.3f) else Color.Transparent,
                RoundedCornerShape(4.dp)
            )
            .padding(horizontal = 6.dp, vertical = 2.dp)
    ) {
        Text(
            text = label,
            color = MilitaryTextSecondary,
            fontSize = 9.sp,
            fontFamily = FontFamily.Monospace,
            fontWeight = FontWeight.Normal
        )
        Text(
            text = value,
            color = if (isActive) MilitaryTextPrimary else MilitaryTextSecondary,
            fontSize = 11.sp,
            fontFamily = FontFamily.Monospace,
            fontWeight = FontWeight.Bold
        )
    }
}

@Composable
private fun ConnectionStatusCompact(systemStatus: SystemStatus) {
    val statusColor = when (systemStatus.connectionStatus) {
        ConnectionState.CONNECTED -> StatusConnected
        ConnectionState.CONNECTING -> StatusConnecting
        ConnectionState.DISCONNECTED -> StatusDisconnected
        ConnectionState.ERROR -> StatusError
    }

    val statusText = when (systemStatus.connectionStatus) {
        ConnectionState.CONNECTED -> "CONN"
        ConnectionState.CONNECTING -> "..."
        ConnectionState.DISCONNECTED -> "DISC"
        ConnectionState.ERROR -> "ERR"
    }

    Row(
        verticalAlignment = Alignment.CenterVertically,
        horizontalArrangement = Arrangement.spacedBy(4.dp),
        modifier = Modifier
            .background(
                statusColor.copy(alpha = 0.3f),
                RoundedCornerShape(4.dp)
            )
            .padding(horizontal = 6.dp, vertical = 4.dp)
    ) {
        Box(
            modifier = Modifier
                .size(6.dp)
                .background(statusColor, androidx.compose.foundation.shape.CircleShape)
        )
        Text(
            text = statusText,
            color = MilitaryTextPrimary,
            fontSize = 9.sp,
            fontFamily = FontFamily.Monospace,
            fontWeight = FontWeight.Bold
        )
    }
}

@Composable
private fun VideoFeedSection(
    detectedTargets: List<DetectedTarget>,
    modifier: Modifier = Modifier
) {
    TacticalVideoPlayer(
        detectedTargets = detectedTargets,
        onTargetClick = { target ->
            android.util.Log.d("Dashboard", "Target selected: ${target.targetType} at ${target.distance}m")
        },
        modifier = modifier
    )
}
package com.example.mysnipeit.ui.dashboard

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.mysnipeit.data.models.SensorData
import com.example.mysnipeit.data.models.DetectedTarget
import com.example.mysnipeit.data.models.ShootingSolution
import com.example.mysnipeit.data.models.SystemStatus
import com.example.mysnipeit.data.models.ConnectionState
import com.example.mysnipeit.ui.theme.*

@Composable
fun DashboardScreen(
    sensorData: SensorData?,
    detectedTargets: List<DetectedTarget>,
    shootingSolution: ShootingSolution?,
    systemStatus: SystemStatus,
    onConnectClick: () -> Unit,
    onDisconnectClick: () -> Unit,
    onBackClick: () -> Unit = {}
) {
    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(MilitaryDarkBackground)
            .padding(16.dp),
        verticalArrangement = Arrangement.spacedBy(12.dp)
    ) {
        // Header with title and connection status
        HeaderSection(
            systemStatus = systemStatus,
            onConnectClick = onConnectClick,
            onDisconnectClick = onDisconnectClick,
            onBackClick = onBackClick
        )

        Row(
            modifier = Modifier.fillMaxSize(),
            horizontalArrangement = Arrangement.spacedBy(16.dp)
        ) {
            // Left side - Video feed placeholder
            VideoFeedSection(
                modifier = Modifier.weight(2f),
                detectedTargets = detectedTargets
            )

            // Right side - Data panels
            Column(
                modifier = Modifier.weight(1f),
                verticalArrangement = Arrangement.spacedBy(12.dp)
            ) {
                // Shooting solution
                ShootingSolutionPanel(shootingSolution)

                // Sensor data
                SensorDataPanel(sensorData)

                // Targets list
                TargetsPanel(detectedTargets)
            }
        }
    }
}

@Composable
private fun HeaderSection(
    systemStatus: SystemStatus,
    onConnectClick: () -> Unit,
    onDisconnectClick: () -> Unit,
    onBackClick: () -> Unit = {}
) {
    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = Alignment.CenterVertically
    ) {
        // Back button + Title
        Row(
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.spacedBy(16.dp)
        ) {
            Button(
                onClick = onBackClick,
                colors = ButtonDefaults.buttonColors(
                    containerColor = MilitaryBorderColor
                )
            ) {
                Text(
                    text = "← BACK",
                    fontFamily = FontFamily.Monospace,
                    fontWeight = FontWeight.Bold
                )
            }

            Text(
                text = "SNIPER SPOTTER SYSTEM",
                fontSize = 24.sp,
                fontWeight = FontWeight.Bold,
                fontFamily = FontFamily.Monospace,
                color = MilitaryAccentGreen
            )
        }

        Row(
            horizontalArrangement = Arrangement.spacedBy(8.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            // Connection status indicator
            ConnectionStatusIndicator(systemStatus.connectionStatus)

            // Battery indicator
            systemStatus.batteryLevel?.let { battery ->
                Text(
                    text = "$battery%",
                    color = if (battery > 20) MilitaryAccentGreen else MilitaryDangerRed,
                    fontFamily = FontFamily.Monospace,
                    fontWeight = FontWeight.Bold
                )
            }

            // Connection button
            Button(
                onClick = if (systemStatus.connectionStatus == ConnectionState.CONNECTED) {
                    onDisconnectClick
                } else {
                    onConnectClick
                },
                colors = ButtonDefaults.buttonColors(
                    containerColor = if (systemStatus.connectionStatus == ConnectionState.CONNECTED) {
                        MilitaryDangerRed
                    } else {
                        MilitaryAccentGreen
                    }
                )
            ) {
                Text(
                    text = if (systemStatus.connectionStatus == ConnectionState.CONNECTED) "DISCONNECT" else "CONNECT",
                    fontFamily = FontFamily.Monospace,
                    fontWeight = FontWeight.Bold
                )
            }
        }
    }
}

@Composable
private fun VideoFeedSection(
    modifier: Modifier = Modifier,
    detectedTargets: List<DetectedTarget>
) {
    Card(
        modifier = modifier.fillMaxHeight(),
        colors = CardDefaults.cardColors(containerColor = MilitaryCardBackground),
        border = androidx.compose.foundation.BorderStroke(2.dp, MilitaryAccentGreen)
    ) {
        Box(
            modifier = Modifier.fillMaxSize(),
            contentAlignment = Alignment.Center
        ) {
            // Placeholder for video feed
            Text(
                text = "VIDEO FEED\n\nTargets Detected: ${detectedTargets.size}",
                color = MilitaryTextSecondary,
                fontFamily = FontFamily.Monospace,
                fontSize = 18.sp,
                textAlign = androidx.compose.ui.text.style.TextAlign.Center
            )
        }
    }
}

@Composable
private fun ConnectionStatusIndicator(status: ConnectionState) {
    val color = when (status) {
        ConnectionState.CONNECTED -> StatusConnected
        ConnectionState.CONNECTING -> StatusConnecting
        ConnectionState.DISCONNECTED -> StatusDisconnected
        ConnectionState.ERROR -> StatusError
    }

    Row(
        verticalAlignment = Alignment.CenterVertically,
        horizontalArrangement = Arrangement.spacedBy(4.dp)
    ) {
        Box(
            modifier = Modifier
                .size(12.dp)
                .background(color, androidx.compose.foundation.shape.CircleShape)
        )
        Text(
            text = status.name,
            color = color,
            fontFamily = FontFamily.Monospace,
            fontWeight = FontWeight.Bold,
            fontSize = 12.sp
        )
    }
}

@Composable
private fun ShootingSolutionPanel(solution: ShootingSolution?) {
    MilitaryPanel(title = "SHOOTING SOLUTION") {
        if (solution != null) {
            Text("Azimuth: ${String.format("%.1f°", solution.azimuth)}", color = MilitaryTextPrimary)
            Text("Elevation: ${String.format("%.1f°", solution.elevation)}", color = MilitaryTextPrimary)
            Text("Confidence: ${String.format("%.0f%%", solution.confidence * 100)}", color = MilitaryTextPrimary)
        } else {
            Text("NO SOLUTION", color = MilitaryTextSecondary)
        }
    }
}

@Composable
private fun SensorDataPanel(data: SensorData?) {
    MilitaryPanel(title = "ENVIRONMENTAL") {
        if (data != null) {
            Text("Temp: ${String.format("%.1f°C", data.temperature)}", color = MilitaryTextPrimary)
            Text("Humidity: ${String.format("%.1f%%", data.humidity)}", color = MilitaryTextPrimary)
            Text("Wind: ${String.format("%.1f m/s @ %.0f°", data.windSpeed, data.windDirection)}", color = MilitaryTextPrimary)
            Text("Range: ${String.format("%.0fm", data.rangefinderDistance)}", color = MilitaryTextPrimary)
        } else {
            Text("NO DATA", color = MilitaryTextSecondary)
        }
    }
}

@Composable
private fun TargetsPanel(targets: List<DetectedTarget>) {
    MilitaryPanel(title = "TARGETS") {
        if (targets.isNotEmpty()) {
            targets.take(3).forEach { target ->
                Text(
                    text = "${target.targetType.name} - ${String.format("%.0fm", target.distance)}",
                    color = MilitaryTextPrimary,
                    fontSize = 12.sp
                )
            }
        } else {
            Text("NO TARGETS", color = MilitaryTextSecondary)
        }
    }
}

@Composable
private fun MilitaryPanel(
    title: String,
    content: @Composable ColumnScope.() -> Unit
) {
    Card(
        colors = CardDefaults.cardColors(containerColor = MilitaryCardBackground),
        border = androidx.compose.foundation.BorderStroke(1.dp, MilitaryBorderColor)
    ) {
        Column(
            modifier = Modifier.padding(12.dp),
            verticalArrangement = Arrangement.spacedBy(4.dp)
        ) {
            Text(
                text = title,
                color = MilitaryAccentGreen,
                fontFamily = FontFamily.Monospace,
                fontWeight = FontWeight.Bold,
                fontSize = 14.sp
            )
            content()
        }
    }
}
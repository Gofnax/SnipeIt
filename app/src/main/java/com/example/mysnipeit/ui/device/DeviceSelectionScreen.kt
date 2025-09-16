package com.example.mysnipeit.ui.device

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.mysnipeit.data.models.Device
import com.example.mysnipeit.data.models.DeviceStatus
import com.example.mysnipeit.ui.theme.*

@Composable
fun DeviceSelectionScreen(
    devices: List<Device>,
    onDeviceSelected: (Device) -> Unit,
    onScanClick: () -> Unit
) {
    Box(
        modifier = Modifier
            .fillMaxSize()
            .background(MilitaryDarkBackground)
    ) {
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(24.dp),
            horizontalAlignment = Alignment.CenterHorizontally
        ) {
            // App logo and title
            Column(
                horizontalAlignment = Alignment.CenterHorizontally,
                modifier = Modifier.padding(vertical = 32.dp)
            ) {
                // Logo placeholder (you can add an actual icon later)
                Box(
                    modifier = Modifier
                        .size(120.dp)
                        .background(MilitaryAccentGreen.copy(alpha = 0.2f), RoundedCornerShape(60.dp)),
                    contentAlignment = Alignment.Center
                ) {
                    Text(
                        text = "⌖",
                        fontSize = 48.sp,
                        color = MilitaryAccentGreen
                    )
                }

                Spacer(modifier = Modifier.height(16.dp))

                Text(
                    text = "SnipeIt",
                    fontSize = 32.sp,
                    fontWeight = FontWeight.Bold,
                    color = MilitaryTextPrimary,
                    fontFamily = FontFamily.Monospace
                )
            }

            Spacer(modifier = Modifier.height(32.dp))

            // Device list
            Text(
                text = "Select a Device",
                fontSize = 20.sp,
                color = MilitaryTextPrimary,
                fontFamily = FontFamily.Monospace,
                modifier = Modifier.padding(bottom = 16.dp)
            )

            LazyColumn(
                verticalArrangement = Arrangement.spacedBy(12.dp),
                modifier = Modifier.weight(1f)
            ) {
                items(devices) { device ->
                    DeviceCard(
                        device = device,
                        onDeviceClick = { onDeviceSelected(device) }
                    )
                }
            }

            Spacer(modifier = Modifier.height(24.dp))

            // Scan button
            Button(
                onClick = onScanClick,
                modifier = Modifier
                    .fillMaxWidth()
                    .height(56.dp),
                colors = ButtonDefaults.buttonColors(
                    containerColor = MilitaryWarningAmber
                ),
                shape = RoundedCornerShape(28.dp)
            ) {
                Text(
                    text = "Begin Scan",
                    fontSize = 18.sp,
                    fontWeight = FontWeight.Bold,
                    color = Color.Black,
                    fontFamily = FontFamily.Monospace
                )
            }
        }
    }
}

@Composable
private fun DeviceCard(
    device: Device,
    onDeviceClick: () -> Unit
) {
    val statusColor = when (device.status) {
        DeviceStatus.ACTIVE -> StatusConnected
        DeviceStatus.SCANNING -> StatusConnecting
        DeviceStatus.INACTIVE -> StatusDisconnected
        DeviceStatus.ERROR -> StatusError
    }

    Card(
        modifier = Modifier
            .fillMaxWidth()
            .clickable { onDeviceClick() },
        colors = CardDefaults.cardColors(
            containerColor = if (device.status == DeviceStatus.ACTIVE) {
                MilitaryAccentGreen.copy(alpha = 0.1f)
            } else {
                MilitaryCardBackground
            }
        ),
        border = androidx.compose.foundation.BorderStroke(
            2.dp,
            if (device.status == DeviceStatus.ACTIVE) MilitaryAccentGreen else MilitaryBorderColor
        )
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(20.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            // Device info
            Column(
                modifier = Modifier.weight(1f)
            ) {
                Text(
                    text = device.name,
                    fontSize = 18.sp,
                    fontWeight = FontWeight.Bold,
                    color = MilitaryTextPrimary,
                    fontFamily = FontFamily.Monospace
                )

                Text(
                    text = "Location: ${device.location}",
                    fontSize = 14.sp,
                    color = MilitaryTextSecondary,
                    fontFamily = FontFamily.Monospace
                )

                Text(
                    text = "Longitude: ${device.longitude}",
                    fontSize = 14.sp,
                    color = MilitaryTextSecondary,
                    fontFamily = FontFamily.Monospace
                )

                Text(
                    text = "Latitude: ${device.latitude}",
                    fontSize = 14.sp,
                    color = MilitaryTextSecondary,
                    fontFamily = FontFamily.Monospace
                )

                Row(
                    verticalAlignment = Alignment.CenterVertically,
                    horizontalArrangement = Arrangement.spacedBy(8.dp)
                ) {
                    Text(
                        text = "Status:",
                        fontSize = 14.sp,
                        color = MilitaryTextSecondary,
                        fontFamily = FontFamily.Monospace
                    )

                    Text(
                        text = device.status.name,
                        fontSize = 14.sp,
                        color = statusColor,
                        fontFamily = FontFamily.Monospace,
                        fontWeight = FontWeight.Bold
                    )
                }

                Text(
                    text = "Battery: ${device.batteryLevel}%",
                    fontSize = 14.sp,
                    color = if (device.batteryLevel > 20) MilitaryAccentGreen else MilitaryDangerRed,
                    fontFamily = FontFamily.Monospace
                )
            }

            // Status indicator
            Box(
                modifier = Modifier
                    .size(16.dp)
                    .background(statusColor, androidx.compose.foundation.shape.CircleShape)
            )
        }
    }
}


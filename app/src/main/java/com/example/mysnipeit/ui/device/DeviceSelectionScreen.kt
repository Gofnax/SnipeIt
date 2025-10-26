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
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.compose.ui.window.Dialog
import com.example.mysnipeit.data.models.Device
import com.example.mysnipeit.data.models.DeviceStatus
import com.example.mysnipeit.ui.theme.*

@Composable
fun DeviceSelectionScreen(
    devices: List<Device>,
    onDeviceSelected: (Device) -> Unit,
    onBackClick: () -> Unit
) {
    var selectedDevice by remember { mutableStateOf<Device?>(null) }

    Box(
        modifier = Modifier
            .fillMaxSize()
            .background(MilitaryDarkBackground)
    ) {
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(24.dp)
        ) {
            // Header with back button
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(bottom = 24.dp),
                verticalAlignment = Alignment.CenterVertically,
                horizontalArrangement = Arrangement.spacedBy(16.dp)
            ) {
                Button(
                    onClick = onBackClick,
                    colors = ButtonDefaults.buttonColors(
                        containerColor = MilitaryBorderColor
                    ),
                    modifier = Modifier.size(48.dp),
                    contentPadding = PaddingValues(0.dp)
                ) {
                    Text(
                        text = "←",
                        fontSize = 24.sp,
                        fontWeight = FontWeight.Bold,
                        color = MilitaryTextPrimary
                    )
                }

                Text(
                    text = "DEVICE LIST",
                    fontSize = 24.sp,
                    fontWeight = FontWeight.Bold,
                    color = MilitaryTextPrimary,
                    fontFamily = FontFamily.Monospace
                )
            }

            // Device count
            Text(
                text = "${devices.size} DEVICES AVAILABLE",
                fontSize = 14.sp,
                color = MilitaryTextSecondary,
                fontFamily = FontFamily.Monospace,
                modifier = Modifier.padding(bottom = 16.dp)
            )

            // Device list
            LazyColumn(
                verticalArrangement = Arrangement.spacedBy(12.dp),
                modifier = Modifier.weight(1f)
            ) {
                items(devices) { device ->
                    DeviceCard(
                        device = device,
                        onDeviceClick = { selectedDevice = device }
                    )
                }
            }
        }

        // Show dialog when device is selected
        selectedDevice?.let { device ->
            DeviceDetailsDialog(
                device = device,
                onDismiss = { selectedDevice = null },
                onConnect = {
                    onDeviceSelected(device)
                    selectedDevice = null
                }
            )
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

            Box(
                modifier = Modifier
                    .size(16.dp)
                    .background(statusColor, androidx.compose.foundation.shape.CircleShape)
            )
        }
    }
}

@Composable
private fun DeviceDetailsDialog(
    device: Device,
    onDismiss: () -> Unit,
    onConnect: () -> Unit
) {
    Dialog(onDismissRequest = onDismiss) {
        Card(
            colors = CardDefaults.cardColors(
                containerColor = MilitaryCardBackground
            ),
            shape = RoundedCornerShape(12.dp),
            border = androidx.compose.foundation.BorderStroke(2.dp, MilitaryAccentGreen)
        ) {
            Column(
                modifier = Modifier
                    .padding(24.dp)
                    .widthIn(min = 300.dp),
                horizontalAlignment = Alignment.CenterHorizontally
            ) {
                Text(
                    text = device.name,
                    fontSize = 24.sp,
                    fontWeight = FontWeight.Bold,
                    color = MilitaryTextPrimary,
                    fontFamily = FontFamily.Monospace
                )

                Spacer(modifier = Modifier.height(24.dp))

                // Device details
                DeviceDetailRow("Location", device.location)
                DeviceDetailRow("Longitude", device.longitude.toString())
                DeviceDetailRow("Latitude", device.latitude.toString())
                DeviceDetailRow("Status", device.status.name)
                DeviceDetailRow("Battery", "${device.batteryLevel}%")
                DeviceDetailRow("IP Address", device.ipAddress)

                Spacer(modifier = Modifier.height(24.dp))

                // Action buttons
                Row(
                    horizontalArrangement = Arrangement.spacedBy(12.dp)
                ) {
                    OutlinedButton(
                        onClick = onDismiss,
                        colors = ButtonDefaults.outlinedButtonColors(
                            contentColor = MilitaryTextPrimary
                        ),
                        border = androidx.compose.foundation.BorderStroke(1.dp, MilitaryBorderColor)
                    ) {
                        Text(
                            text = "CANCEL",
                            fontFamily = FontFamily.Monospace,
                            fontWeight = FontWeight.Bold
                        )
                    }

                    Button(
                        onClick = onConnect,
                        colors = ButtonDefaults.buttonColors(
                            containerColor = MilitaryAccentGreen
                        )
                    ) {
                        Text(
                            text = "CONNECT",
                            fontFamily = FontFamily.Monospace,
                            fontWeight = FontWeight.Bold,
                            color = Color.Black
                        )
                    }
                }
            }
        }
    }
}

@Composable
private fun DeviceDetailRow(label: String, value: String) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .padding(vertical = 4.dp),
        horizontalArrangement = Arrangement.SpaceBetween
    ) {
        Text(
            text = "$label:",
            fontSize = 14.sp,
            color = MilitaryTextSecondary,
            fontFamily = FontFamily.Monospace
        )
        Text(
            text = value,
            fontSize = 14.sp,
            color = MilitaryTextPrimary,
            fontFamily = FontFamily.Monospace,
            fontWeight = FontWeight.Bold
        )
    }
}
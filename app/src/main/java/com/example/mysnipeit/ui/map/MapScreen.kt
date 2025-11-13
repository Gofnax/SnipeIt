package com.example.mysnipeit.ui.map

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
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
import com.google.android.gms.maps.model.CameraPosition
import com.google.android.gms.maps.model.LatLng
import com.google.maps.android.compose.*
import kotlin.random.Random

@Composable
fun MapScreen(
    devices: List<Device>,
    userLocation: LatLng,
    onDeviceSelected: (Device) -> Unit,
    onBackClick: () -> Unit
) {
    var selectedDevice by remember { mutableStateOf<Device?>(null) }

    val cameraPositionState = rememberCameraPositionState {
        position = CameraPosition.fromLatLngZoom(userLocation, 14f)
    }

    Box(
        modifier = Modifier
            .fillMaxSize()
            .background(MilitaryDarkBackground)
    ) {
        // Google Map
        GoogleMap(
            modifier = Modifier.fillMaxSize(),
            cameraPositionState = cameraPositionState,
            properties = MapProperties(
                mapType = MapType.SATELLITE,
                isMyLocationEnabled = false
            ),
            uiSettings = MapUiSettings(
                zoomControlsEnabled = false,
                myLocationButtonEnabled = false
            )
        ) {
            // User location marker
            Marker(
                state = MarkerState(position = userLocation),
                title = "Your Location",
                icon = com.google.android.gms.maps.model.BitmapDescriptorFactory.defaultMarker(
                    com.google.android.gms.maps.model.BitmapDescriptorFactory.HUE_BLUE
                )
            )

            // Device markers
            devices.forEach { device ->
                val deviceLocation = LatLng(device.latitude, device.longitude)
                val markerColor = when (device.status) {
                    DeviceStatus.ACTIVE -> com.google.android.gms.maps.model.BitmapDescriptorFactory.HUE_GREEN
                    else -> com.google.android.gms.maps.model.BitmapDescriptorFactory.HUE_RED
                }

                Marker(
                    state = MarkerState(position = deviceLocation),
                    title = device.name,
                    snippet = device.status.name,
                    icon = com.google.android.gms.maps.model.BitmapDescriptorFactory.defaultMarker(markerColor),
                    onClick = {
                        selectedDevice = device
                        true
                    }
                )
            }
        }

        // Back button overlay
        Button(
            onClick = onBackClick,
            modifier = Modifier
                .align(Alignment.TopStart)
                .padding(24.dp)
                .size(56.dp),
            colors = ButtonDefaults.buttonColors(
                containerColor = MilitaryCardBackground.copy(alpha = 0.95f)
            ),
            contentPadding = PaddingValues(0.dp)
        ) {
            Text(
                text = "←",
                fontSize = 24.sp,
                fontWeight = FontWeight.Bold,
                color = MilitaryTextPrimary
            )
        }

        // Map title overlay
        Card(
            modifier = Modifier
                .align(Alignment.TopCenter)
                .padding(24.dp),
            colors = CardDefaults.cardColors(
                containerColor = MilitaryCardBackground.copy(alpha = 0.95f)
            )
        ) {
            Text(
                text = "MAP VIEW - ${devices.size} DEVICES",
                fontSize = 16.sp,
                fontWeight = FontWeight.Bold,
                color = MilitaryTextPrimary,
                fontFamily = FontFamily.Monospace,
                modifier = Modifier.padding(16.dp)
            )
        }

        // Device details dialog
        selectedDevice?.let { device ->
            DeviceMapDialog(
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
private fun DeviceMapDialog(
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

                Spacer(modifier = Modifier.height(16.dp))

                // Status badge
                val statusColor = when (device.status) {
                    DeviceStatus.ACTIVE -> StatusConnected
                    DeviceStatus.SCANNING -> StatusConnecting
                    DeviceStatus.INACTIVE -> StatusDisconnected
                    DeviceStatus.ERROR -> StatusError
                }

                Surface(
                    color = statusColor.copy(alpha = 0.2f),
                    shape = RoundedCornerShape(16.dp),
                    border = androidx.compose.foundation.BorderStroke(1.dp, statusColor)
                ) {
                    Text(
                        text = device.status.name,
                        fontSize = 12.sp,
                        fontWeight = FontWeight.Bold,
                        color = statusColor,
                        fontFamily = FontFamily.Monospace,
                        modifier = Modifier.padding(horizontal = 12.dp, vertical = 6.dp)
                    )
                }

                Spacer(modifier = Modifier.height(16.dp))

                // Device details
                DeviceDetailRow("Location", device.location)
                DeviceDetailRow("Distance", calculateDistance(device) + " km")
                DeviceDetailRow("Battery", "${device.batteryLevel}%")
                DeviceDetailRow("IP", device.ipAddress)

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
                            text = "CLOSE",
                            fontFamily = FontFamily.Monospace,
                            fontWeight = FontWeight.Bold
                        )
                    }

                    Button(
                        onClick = onConnect,
                        enabled = device.status == DeviceStatus.ACTIVE,
                        colors = ButtonDefaults.buttonColors(
                            containerColor = if (device.status == DeviceStatus.ACTIVE) {
                                MilitaryAccentGreen  // Green for active
                            } else {
                                Color.Gray  // Gray for inactive
                            },
                            disabledContainerColor = Color.Gray.copy(alpha = 0.5f)
                        )
                    ) {
                        Text(
                            text = if (device.status == DeviceStatus.ACTIVE) {
                                "CONNECT"
                            } else {
                                "DEVICE OFFLINE"
                            },
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

private fun calculateDistance(device: Device): String {
    // Simple mock distance calculation
    return String.format("%.2f", Random.nextDouble(0.5, 5.0))
}
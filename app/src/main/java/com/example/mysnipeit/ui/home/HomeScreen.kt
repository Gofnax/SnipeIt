package com.example.mysnipeit.ui.home

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.List
import androidx.compose.material.icons.filled.LocationOn
import androidx.compose.material3.*
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.mysnipeit.ui.theme.*

@Composable
fun HomeScreen(
    onDeviceListClick: () -> Unit,
    onMapClick: () -> Unit
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
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.Center
        ) {
            // App Title/Logo
            Column(
                horizontalAlignment = Alignment.CenterHorizontally,
                modifier = Modifier.padding(bottom = 64.dp)
            ) {
                Box(
                    modifier = Modifier
                        .size(120.dp)
                        .background(
                            MilitaryAccentGreen.copy(alpha = 0.2f),
                            RoundedCornerShape(60.dp)
                        ),
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

                Text(
                    text = "SPOTTER TACTICAL SYSTEM",
                    fontSize = 16.sp,
                    color = MilitaryTextSecondary,
                    fontFamily = FontFamily.Monospace
                )
            }

            // Menu Buttons
            MenuButton(
                icon = Icons.Default.List,
                text = "DEVICE LIST",
                description = "View all available devices",
                onClick = onDeviceListClick
            )

            Spacer(modifier = Modifier.height(24.dp))

            MenuButton(
                icon = Icons.Default.LocationOn,
                text = "MAP VIEW",
                description = "Locate devices on map",
                onClick = onMapClick
            )
        }
    }
}

@Composable
private fun MenuButton(
    icon: androidx.compose.ui.graphics.vector.ImageVector,
    text: String,
    description: String,
    onClick: () -> Unit
) {
    Button(
        onClick = onClick,
        modifier = Modifier
            .fillMaxWidth()
            .height(120.dp),
        colors = ButtonDefaults.buttonColors(
            containerColor = MilitaryCardBackground
        ),
        shape = RoundedCornerShape(8.dp)
    ) {
        Row(
            modifier = Modifier.fillMaxWidth(),
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.spacedBy(24.dp)
        ) {
            Icon(
                imageVector = icon,
                contentDescription = text,
                tint = MilitaryAccentGreen,
                modifier = Modifier.size(48.dp)
            )

            Column(
                horizontalAlignment = Alignment.Start
            ) {
                Text(
                    text = text,
                    fontSize = 20.sp,
                    fontWeight = FontWeight.Bold,
                    color = MilitaryTextPrimary,
                    fontFamily = FontFamily.Monospace
                )

                Text(
                    text = description,
                    fontSize = 14.sp,
                    color = MilitaryTextSecondary,
                    fontFamily = FontFamily.Monospace
                )
            }
        }
    }
}
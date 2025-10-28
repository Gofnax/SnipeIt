package com.example.mysnipeit.ui.home

import android.util.Log
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.mysnipeit.ui.theme.*

@Composable
fun HomeScreen(
    onDeviceListClick: () -> Unit,
    onMapClick: () -> Unit,
    onDiagnosticsClick: () -> Unit = {}
) {
    // Debug logging
    Log.d("HomeScreen", "HomeScreen composing...")

    Box(
        modifier = Modifier
            .fillMaxSize()
            .background(MilitaryDarkBackground)
    ) {
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(32.dp),
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.Center
        ) {
            Text(
                text = "SnipeIt",
                fontSize = 64.sp,
                fontWeight = FontWeight.Bold,
                color = MilitaryAccentGreen,
                fontFamily = FontFamily.Monospace,
                modifier = Modifier.padding(bottom = 8.dp)
            )

            Text(
                text = "Smart Spotter System",
                fontSize = 20.sp,
                fontWeight = FontWeight.Light,
                color = MilitaryTextPrimary,
                fontFamily = FontFamily.Monospace,
                modifier = Modifier.padding(bottom = 64.dp)
            )

            Spacer(modifier = Modifier.weight(1f))

            // Device List Button
            Button(
                onClick = {
                    Log.d("HomeScreen", "Device List clicked")
                    onDeviceListClick()
                },
                modifier = Modifier
                    .fillMaxWidth()
                    .height(70.dp)
                    .padding(vertical = 8.dp),
                colors = ButtonDefaults.buttonColors(
                    containerColor = MilitaryAccentGreen
                )
            ) {
                Text(
                    text = "DEVICE LIST",
                    fontSize = 20.sp,
                    fontWeight = FontWeight.Bold,
                    fontFamily = FontFamily.Monospace,
                    color = Color.Black
                )
            }

            // Map Button
            Button(
                onClick = {
                    Log.d("HomeScreen", "Map clicked")
                    onMapClick()
                },
                modifier = Modifier
                    .fillMaxWidth()
                    .height(70.dp)
                    .padding(vertical = 8.dp),
                colors = ButtonDefaults.buttonColors(
                    containerColor = MilitaryCardBackground
                ),
                border = androidx.compose.foundation.BorderStroke(2.dp, MilitaryAccentGreen)
            ) {
                Text(
                    text = "MAP VIEW",
                    fontSize = 20.sp,
                    fontWeight = FontWeight.Bold,
                    fontFamily = FontFamily.Monospace,
                    color = MilitaryTextPrimary
                )
            }

            // Diagnostics Button - WITH DEBUG LOGGING
            Button(
                onClick = {
                    onDiagnosticsClick()
                                    },
                modifier = Modifier
                    .fillMaxWidth()
                    .height(70.dp)
                    .padding(vertical = 8.dp),
                colors = ButtonDefaults.buttonColors(
                    containerColor = MilitaryBorderColor
                ),
                border = androidx.compose.foundation.BorderStroke(2.dp, MilitaryWarningAmber)
            ) {
                Row(
                    horizontalArrangement = Arrangement.Center,
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Text(
                        text = "⚙ ",
                        fontSize = 20.sp,
                        color = MilitaryWarningAmber
                    )
                    Text(
                        text = "DIAGNOSTICS",
                        fontSize = 20.sp,
                        fontWeight = FontWeight.Bold,
                        fontFamily = FontFamily.Monospace,
                        color = MilitaryTextPrimary
                    )
                }
            }

            Spacer(modifier = Modifier.weight(1f))

//            Text(
//                text = "TACTICAL OPERATIONS",
//                fontSize = 12.sp,
//                color = MilitaryTextSecondary,
//                fontFamily = FontFamily.Monospace,
//                modifier = Modifier.padding(top = 16.dp)
//            )
        }
    }
}
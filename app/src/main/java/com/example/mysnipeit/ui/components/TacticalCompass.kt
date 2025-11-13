package com.example.mysnipeit.ui.components

import androidx.compose.animation.core.*
import androidx.compose.foundation.Canvas
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.Text
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.Path
import androidx.compose.ui.graphics.StrokeCap
import androidx.compose.ui.graphics.drawscope.Stroke
import androidx.compose.ui.graphics.drawscope.rotate
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import kotlin.math.cos
import kotlin.math.sin

/**
 * 3D Tactical Compass
 * Shows azimuth (horizontal) and elevation (vertical) angles
 */
@Composable
fun TacticalCompass(
    azimuth: Double,      // 0-360°
    elevation: Double,    // -90 to +90°
    confidence: Float,    // 0.0-1.0
    modifier: Modifier = Modifier
) {
    // Animate the arrow rotation
    val animatedAzimuth by animateFloatAsState(
        targetValue = azimuth.toFloat(),
        animationSpec = tween(durationMillis = 300, easing = FastOutSlowInEasing),
        label = "azimuth"
    )

    // Animate the elevation
    val animatedElevation by animateFloatAsState(
        targetValue = elevation.toFloat(),
        animationSpec = tween(durationMillis = 300, easing = FastOutSlowInEasing),
        label = "elevation"
    )

    Box(
        modifier = modifier
            .size(160.dp)
            .background(
                color = Color.Black.copy(alpha = 0.8f),
                shape = RoundedCornerShape(8.dp)
            )
            .padding(12.dp)
    ) {
        Column(
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.Center,
            modifier = Modifier.fillMaxSize()
        ) {
            // Compass ring with arrow and cardinal directions
            Box(
                modifier = Modifier.size(100.dp),
                contentAlignment = Alignment.Center
            ) {
                // Canvas for compass ring and arrow
                Canvas(modifier = Modifier.fillMaxSize()) {
                    val center = Offset(size.width / 2, size.height / 2)
                    val radius = size.minDimension / 2.5f

                    // Color based on confidence
                    val arrowColor = when {
                        confidence > 0.8f -> Color(0xFF00FF41)  // Bright green
                        confidence > 0.6f -> Color(0xFFFFAA00)  // Orange
                        else -> Color(0xFFFF4444)               // Red
                    }

                    // Draw compass ring
                    drawCircle(
                        color = Color(0xFF00FF41).copy(alpha = 0.3f),
                        radius = radius,
                        center = center,
                        style = Stroke(width = 2f)
                    )

                    // Draw azimuth arrow (rotated)
                    rotate(degrees = animatedAzimuth, pivot = center) {
                        // Arrow shaft
                        drawLine(
                            color = arrowColor,
                            start = center,
                            end = Offset(center.x, center.y - radius * 0.8f),
                            strokeWidth = 4f,
                            cap = StrokeCap.Round
                        )

                        // Arrow head
                        val arrowPath = Path().apply {
                            val arrowTip = Offset(center.x, center.y - radius * 0.8f)
                            val arrowSize = 10f

                            moveTo(arrowTip.x, arrowTip.y)
                            lineTo(arrowTip.x - arrowSize, arrowTip.y + arrowSize * 1.5f)
                            lineTo(arrowTip.x + arrowSize, arrowTip.y + arrowSize * 1.5f)
                            close()
                        }

                        drawPath(
                            path = arrowPath,
                            color = arrowColor
                        )
                    }

                    // Draw elevation indicator (3D angle representation)
                    val elevationRadius = radius * 0.4f
                    val elevationStartAngle = 180f - (animatedElevation / 2f)
                    val elevationSweep = animatedElevation

                    if (animatedElevation.toInt() != 0) {
                        drawArc(
                            color = arrowColor.copy(alpha = 0.6f),
                            startAngle = elevationStartAngle,
                            sweepAngle = elevationSweep,
                            useCenter = false,
                            topLeft = Offset(
                                center.x - elevationRadius,
                                center.y - elevationRadius
                            ),
                            size = androidx.compose.ui.geometry.Size(
                                elevationRadius * 2,
                                elevationRadius * 2
                            ),
                            style = Stroke(width = 3f)
                        )
                    }

                    // Center dot
                    drawCircle(
                        color = arrowColor,
                        radius = 4f,
                        center = center
                    )
                }

                //  Cardinal directions using Compose Text instead of Canvas.drawText
                // North
                Text(
                    text = "N",
                    color = Color(0xFF00FF41),
                    fontSize = 16.sp,
                    fontFamily = FontFamily.Monospace,
                    fontWeight = FontWeight.Bold,
                    modifier = Modifier
                        .align(Alignment.TopCenter)
                        .offset(y = (-10).dp)
                )

                // East
                Text(
                    text = "E",
                    color = Color(0xFF00FF41),
                    fontSize = 16.sp,
                    fontFamily = FontFamily.Monospace,
                    fontWeight = FontWeight.Bold,
                    modifier = Modifier
                        .align(Alignment.CenterEnd)
                        .offset(x = 6.dp)
                )

                // South
                Text(
                    text = "S",
                    color = Color(0xFF00FF41),
                    fontSize = 16.sp,
                    fontFamily = FontFamily.Monospace,
                    fontWeight = FontWeight.Bold,
                    modifier = Modifier
                        .align(Alignment.BottomCenter)
                        .offset(y = 10.dp)
                )

                // West
                Text(
                    text = "W",
                    color = Color(0xFF00FF41),
                    fontSize = 16.sp,
                    fontFamily = FontFamily.Monospace,
                    fontWeight = FontWeight.Bold,
                    modifier = Modifier
                        .align(Alignment.CenterStart)
                        .offset(x = (-6).dp)
                )
            }

            Spacer(modifier = Modifier.height(6.dp))

            // Numerical displays
            Row(
                horizontalArrangement = Arrangement.SpaceEvenly,
                modifier = Modifier.fillMaxWidth()
            ) {
                // Azimuth value
                Column(horizontalAlignment = Alignment.CenterHorizontally) {
                    Text(
                        text = "AZ",
                        color = Color(0xFF00FF41).copy(alpha = 0.6f),
                        fontSize = 10.sp,
                        fontFamily = FontFamily.Monospace
                    )
                    Text(
                        text = "${azimuth.toInt()}°",
                        color = Color(0xFF00FF41),
                        fontSize = 13.sp,
                        fontWeight = FontWeight.Bold,
                        fontFamily = FontFamily.Monospace
                    )
                }

                // Elevation value
                Column(horizontalAlignment = Alignment.CenterHorizontally) {
                    Text(
                        text = "EL",
                        color = Color(0xFF00FF41).copy(alpha = 0.6f),
                        fontSize = 10.sp,
                        fontFamily = FontFamily.Monospace
                    )
                    Text(
                        text = "${if (elevation > 0) "+" else ""}${elevation.toInt()}°",
                        color = Color(0xFF00FF41),
                        fontSize = 13.sp,
                        fontWeight = FontWeight.Bold,
                        fontFamily = FontFamily.Monospace
                    )
                }
            }
        }
    }
}
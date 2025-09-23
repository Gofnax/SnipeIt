package com.example.mysnipeit.ui.dashboard

import androidx.compose.foundation.Canvas
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.Path
import androidx.compose.ui.graphics.drawscope.DrawScope
import androidx.compose.ui.graphics.drawscope.Stroke
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.mysnipeit.data.models.DetectedTarget
import com.example.mysnipeit.data.models.TargetType
import com.example.mysnipeit.ui.theme.*
import kotlinx.coroutines.delay
import kotlin.math.cos
import kotlin.math.sin
import kotlin.random.Random

@Composable
fun MockVideoFeed(
    detectedTargets: List<DetectedTarget>,
    onTargetClick: (DetectedTarget) -> Unit = {},
    modifier: Modifier = Modifier
) {
    var scanlinePosition by remember { mutableStateOf(0f) }

    // Animate scanning line
    LaunchedEffect(Unit) {
        while (true) {
            scanlinePosition = 0f
            while (scanlinePosition < 1f) {
                scanlinePosition += 0.002f
                delay(16) // ~60 FPS
            }
            delay(1000) // Pause before next scan
        }
    }

    Box(
        modifier = modifier
            .fillMaxSize()
            .background(Color.Black)
            .border(2.dp, MilitaryAccentGreen)
    ) {
        // Simulated terrain background
        TerrainBackground()

        // Crosshair overlay
        CrosshairOverlay()

        // Scanning line effect
        ScanLineOverlay(scanlinePosition)

        // Target markers
        detectedTargets.forEach { target ->
            TargetMarker(
                target = target,
                onClick = { onTargetClick(target) },
                modifier = Modifier.align(Alignment.Center) // Simplified positioning
            )
        }

        // HUD Elements
        HudOverlay(detectedTargets.size)

        // Video status overlay
        VideoStatusOverlay(
            modifier = Modifier.align(Alignment.TopStart)
        )
    }
}

@Composable
private fun TerrainBackground() {
    Canvas(modifier = Modifier.fillMaxSize()) {
        // Draw simulated terrain with random noise pattern
        val width = size.width
        val height = size.height

        // Draw ground texture
        for (x in 0 until width.toInt() step 20) {
            for (y in 0 until height.toInt() step 20) {
                val brightness = Random.nextFloat() * 0.3f + 0.1f
                drawRect(
                    color = Color(brightness, brightness * 0.8f, brightness * 0.6f, 0.3f),
                    topLeft = Offset(x.toFloat(), y.toFloat()),
                    size = androidx.compose.ui.geometry.Size(20f, 20f)
                )
            }
        }

        // Draw horizon line
        drawLine(
            color = Color(0.4f, 0.4f, 0.4f, 0.6f),
            start = Offset(0f, height * 0.4f),
            end = Offset(width, height * 0.4f),
            strokeWidth = 2f
        )

        // Draw distant hills
        val hillPath = Path()
        hillPath.moveTo(0f, height * 0.4f)
        for (x in 0 until width.toInt() step 50) {
            val hillHeight = sin(x / 100.0) * 20 + height * 0.4f
            hillPath.lineTo(x.toFloat(), hillHeight.toFloat())
        }
        hillPath.lineTo(width, height * 0.4f)

        drawPath(
            path = hillPath,
            color = Color(0.2f, 0.3f, 0.2f, 0.5f),
            style = Stroke(width = 1f)
        )
    }
}

@Composable
private fun CrosshairOverlay() {
    Canvas(modifier = Modifier.fillMaxSize()) {
        val centerX = size.width / 2
        val centerY = size.height / 2
        val crosshairSize = 40f

        // Main crosshair
        drawLine(
            color = MilitaryAccentGreen,
            start = Offset(centerX - crosshairSize, centerY),
            end = Offset(centerX + crosshairSize, centerY),
            strokeWidth = 2f
        )
        drawLine(
            color = MilitaryAccentGreen,
            start = Offset(centerX, centerY - crosshairSize),
            end = Offset(centerX, centerY + crosshairSize),
            strokeWidth = 2f
        )

        // Center dot
        drawCircle(
            color = MilitaryAccentGreen,
            radius = 3f,
            center = Offset(centerX, centerY)
        )

        // Range circles
        for (i in 1..3) {
            drawCircle(
                color = MilitaryAccentGreen,
                radius = crosshairSize * i,
                center = Offset(centerX, centerY),
                style = Stroke(width = 1f, pathEffect = androidx.compose.ui.graphics.PathEffect.dashPathEffect(floatArrayOf(5f, 5f)))
            )
        }
    }
}

@Composable
private fun ScanLineOverlay(position: Float) {
    Canvas(modifier = Modifier.fillMaxSize()) {
        val y = size.height * position

        // Scanning line
        drawLine(
            color = MilitaryAccentGreen.copy(alpha = 0.7f),
            start = Offset(0f, y),
            end = Offset(size.width, y),
            strokeWidth = 2f
        )

        // Fade effect
        val fadeHeight = 100f
        for (i in 0 until fadeHeight.toInt() step 2) {
            val alpha = (1f - (i / fadeHeight)) * 0.3f
            drawLine(
                color = MilitaryAccentGreen.copy(alpha = alpha),
                start = Offset(0f, y + i),
                end = Offset(size.width, y + i),
                strokeWidth = 1f
            )
        }
    }
}

@Composable
private fun TargetMarker(
    target: DetectedTarget,
    onClick: () -> Unit,
    modifier: Modifier = Modifier
) {
    val markerColor = when (target.targetType) {
        TargetType.HUMAN -> MilitaryDangerRed
        TargetType.VEHICLE -> MilitaryWarningAmber
        TargetType.STRUCTURE -> Color.Cyan
        TargetType.UNKNOWN -> Color.Gray
    }

    // Animate pulsing effect for high-confidence targets
    var pulseAlpha by remember { mutableStateOf(1f) }

    if (target.confidence > 0.8f) {
        LaunchedEffect(target.id) {
            while (true) {
                pulseAlpha = 0.3f
                delay(500)
                pulseAlpha = 1f
                delay(500)
            }
        }
    }

    Box(
        modifier = modifier
            .offset(
                x = (Random.nextInt(-200, 200)).dp, // Random positioning for demo
                y = (Random.nextInt(-100, 100)).dp
            )
            .clickable { onClick() }
    ) {
        // Target box
        Canvas(modifier = Modifier.size(60.dp)) {
            val boxSize = 50f
            val centerX = size.width / 2
            val centerY = size.height / 2

            // Draw target box
            drawRect(
                color = markerColor.copy(alpha = pulseAlpha),
                topLeft = Offset(centerX - boxSize/2, centerY - boxSize/2),
                size = androidx.compose.ui.geometry.Size(boxSize, boxSize),
                style = Stroke(width = 3f)
            )

            // Draw corner brackets
            val bracketSize = 15f
            val positions = listOf(
                Offset(centerX - boxSize/2, centerY - boxSize/2), // Top-left
                Offset(centerX + boxSize/2 - bracketSize, centerY - boxSize/2), // Top-right
                Offset(centerX - boxSize/2, centerY + boxSize/2 - bracketSize), // Bottom-left
                Offset(centerX + boxSize/2 - bracketSize, centerY + boxSize/2 - bracketSize) // Bottom-right
            )

            positions.forEach { pos ->
                // Horizontal bracket
                drawLine(
                    color = markerColor,
                    start = pos,
                    end = Offset(pos.x + bracketSize, pos.y),
                    strokeWidth = 2f
                )
                // Vertical bracket
                drawLine(
                    color = markerColor,
                    start = pos,
                    end = Offset(pos.x, pos.y + bracketSize),
                    strokeWidth = 2f
                )
            }
        }

        // Target info label
        Card(
            modifier = Modifier
                .align(Alignment.BottomCenter)
                .offset(y = 35.dp),
            colors = CardDefaults.cardColors(
                containerColor = Color.Black.copy(alpha = 0.8f)
            )
        ) {
            Column(
                modifier = Modifier.padding(4.dp),
                horizontalAlignment = Alignment.CenterHorizontally
            ) {
                Text(
                    text = target.targetType.name,
                    color = markerColor,
                    fontSize = 10.sp,
                    fontFamily = FontFamily.Monospace,
                    fontWeight = FontWeight.Bold
                )
                Text(
                    text = "${target.distance.toInt()}m",
                    color = Color.White,
                    fontSize = 8.sp,
                    fontFamily = FontFamily.Monospace
                )
                Text(
                    text = "${(target.confidence * 100).toInt()}%",
                    color = if (target.confidence > 0.8f) MilitaryAccentGreen else MilitaryWarningAmber,
                    fontSize = 8.sp,
                    fontFamily = FontFamily.Monospace
                )
            }
        }
    }
}

@Composable
private fun HudOverlay(targetCount: Int) {
    Box(modifier = Modifier.fillMaxSize()) {
        // Top HUD bar
        Row(
            modifier = Modifier
                .align(Alignment.TopCenter)
                .padding(16.dp)
                .background(Color.Black.copy(alpha = 0.7f))
                .padding(8.dp),
            horizontalArrangement = Arrangement.spacedBy(24.dp)
        ) {
            HudElement("TGT", targetCount.toString())
            HudElement("RNG", "450M")
            HudElement("AZ", "245°")
            HudElement("EL", "12°")
        }

        // Side range markers
        Column(
            modifier = Modifier
                .align(Alignment.CenterEnd)
                .padding(end = 16.dp),
            verticalArrangement = Arrangement.spacedBy(20.dp)
        ) {
            repeat(5) { index ->
                val range = (index + 1) * 200
                Row(
                    verticalAlignment = Alignment.CenterVertically,
                    horizontalArrangement = Arrangement.spacedBy(4.dp)
                ) {
                    Text(
                        text = "${range}m",
                        color = MilitaryTextSecondary,
                        fontSize = 10.sp,
                        fontFamily = FontFamily.Monospace
                    )
                    Box(
                        modifier = Modifier
                            .width(20.dp)
                            .height(2.dp)
                            .background(MilitaryTextSecondary)
                    )
                }
            }
        }
    }
}

@Composable
private fun HudElement(label: String, value: String) {
    Column(horizontalAlignment = Alignment.CenterHorizontally) {
        Text(
            text = label,
            color = MilitaryTextSecondary,
            fontSize = 9.sp,
            fontFamily = FontFamily.Monospace
        )
        Text(
            text = value,
            color = MilitaryAccentGreen,
            fontSize = 12.sp,
            fontFamily = FontFamily.Monospace,
            fontWeight = FontWeight.Bold
        )
    }
}

@Composable
private fun VideoStatusOverlay(modifier: Modifier = Modifier) {
    Column(
        modifier = modifier
            .padding(16.dp)
            .background(Color.Black.copy(alpha = 0.7f))
            .padding(8.dp)
    ) {
        Row(
            horizontalArrangement = Arrangement.spacedBy(4.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Box(
                modifier = Modifier
                    .size(8.dp)
                    .clip(CircleShape)
                    .background(Color.Red)
            )
            Text(
                text = "LIVE",
                color = Color.Red,
                fontSize = 10.sp,
                fontFamily = FontFamily.Monospace,
                fontWeight = FontWeight.Bold
            )
        }

        Text(
            text = "1920x1080 • 30FPS",
            color = MilitaryTextSecondary,
            fontSize = 8.sp,
            fontFamily = FontFamily.Monospace
        )

        Text(
            text = "THERMAL: OFF",
            color = MilitaryTextSecondary,
            fontSize = 8.sp,
            fontFamily = FontFamily.Monospace
        )
    }
}
package com.example.mysnipeit.ui.dashboard


import android.content.Context
import androidx.compose.foundation.Canvas
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.PathEffect
import androidx.compose.ui.graphics.drawscope.Stroke
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.compose.ui.viewinterop.AndroidView
import androidx.media3.common.MediaItem
import androidx.media3.common.Player
import androidx.media3.exoplayer.ExoPlayer
import androidx.media3.ui.PlayerView
import com.example.mysnipeit.R
import com.example.mysnipeit.data.models.DetectedTarget
import com.example.mysnipeit.data.models.TargetType
import com.example.mysnipeit.ui.theme.*
import kotlinx.coroutines.delay

// Temporary brighter color for testing overlays
private val BrightGreen = Color(0xFF00FF00)

@Composable
fun TacticalVideoPlayer(
    detectedTargets: List<DetectedTarget>,
    onTargetClick: (DetectedTarget) -> Unit = {},
    modifier: Modifier = Modifier
) {
    val context = LocalContext.current
    val exoPlayer = remember { createExoPlayer(context) }

    var scanlinePosition by remember { mutableStateOf(0f) }
    var videoTime by remember { mutableStateOf(0L) }

    // Animate scanning line
    LaunchedEffect(Unit) {
        while (true) {
            scanlinePosition = 0f
            while (scanlinePosition < 1f) {
                scanlinePosition += 0.002f
                delay(16) // ~60 FPS
            }
            delay(1000)
        }
    }

    // Track video playback time
    LaunchedEffect(Unit) {
        while (true) {
            videoTime = exoPlayer.currentPosition
            delay(100) // Update every 100ms
        }
    }

    DisposableEffect(Unit) {
        onDispose {
            exoPlayer.release()
        }
    }

    Box(
        modifier = modifier
            .fillMaxSize()
            .background(Color.Black)
    ) {
        // Video player
        AndroidView(
            factory = { ctx ->
                PlayerView(ctx).apply {
                    player = exoPlayer
                    useController = false // Hide default controls
                    setShowBuffering(PlayerView.SHOW_BUFFERING_NEVER)
                }
            },
            modifier = Modifier.fillMaxSize()
        )

        // Tactical overlays on top of video
        Box(modifier = Modifier.fillMaxSize()) {
            // Crosshair overlay
            CrosshairOverlay()

            // Scanning line effect
            ScanLineOverlay(scanlinePosition)

            // Target markers based on video timeline
            SimulatedTargets(videoTime).forEach { target ->
                    TargetMarker(
                        target = target,
                        onClick = { onTargetClick(target) }
                    )

            }

            // Video status overlay
            VideoStatusOverlay(
                modifier = Modifier.align(Alignment.TopStart)
            )

            // Target count indicator
            if (detectedTargets.isNotEmpty()) {
                Text(
                    text = "TARGET LOCK AVAILABLE",
                    color = MilitaryAccentGreen,
                    fontSize = 10.sp,
                    fontFamily = FontFamily.Monospace,
                    fontWeight = FontWeight.Bold,
                    modifier = Modifier
                        .align(Alignment.BottomCenter)
                        .padding(16.dp)
                        .background(
                            MilitaryAccentGreen.copy(alpha = 0.2f),
                            RoundedCornerShape(4.dp)
                        )
                        .padding(horizontal = 8.dp, vertical = 4.dp)
                )
            }
        }
    }
}

private fun createExoPlayer(context: Context): ExoPlayer {
    return ExoPlayer.Builder(context).build().apply {
        val mediaItem = MediaItem.fromUri("android.resource://${context.packageName}/${R.raw.field_video}")
        setMediaItem(mediaItem)
        repeatMode = Player.REPEAT_MODE_ALL // Loop the video
        playWhenReady = true
        prepare()
    }
}

@Composable
private fun SimulatedTargets(videoTime: Long): List<DetectedTarget> {
    // Simulate target detection based on video timeline
    // Adjust these values based on what's visible in YOUR video
    val targets = mutableListOf<DetectedTarget>()

    // Target 1: Appears at 2 seconds
    if (videoTime > 2000) {
        targets.add(
            DetectedTarget(
                id = "T1",
                confidence = 0.85f,
                screenX = 0.3f, // 30% from left
                screenY = 0.4f, // 40% from top
                worldLatitude = 35.093,
                worldLongitude = 32.014,
                distance = 420.0,
                bearing = 245.0,
                targetType = TargetType.STRUCTURE,
                timestamp = System.currentTimeMillis()
            )
        )
    }

    // Target 2: Appears at 8 seconds
    if (videoTime > 8000) {
        targets.add(
            DetectedTarget(
                id = "T2",
                confidence = 0.72f,
                screenX = 0.7f, // 70% from left
                screenY = 0.5f, // 50% from top
                worldLatitude = 35.094,
                worldLongitude = 32.015,
                distance = 580.0,
                bearing = 260.0,
                targetType = TargetType.UNKNOWN,
                timestamp = System.currentTimeMillis()
            )
        )
    }

    // Target 3: Appears at 15 seconds
    if (videoTime > 15000 && videoTime < 25000) {
        targets.add(
            DetectedTarget(
                id = "T3",
                confidence = 0.91f,
                screenX = 0.5f, // Center
                screenY = 0.45f,
                worldLatitude = 35.095,
                worldLongitude = 32.013,
                distance = 350.0,
                bearing = 250.0,
                targetType = TargetType.HUMAN,
                timestamp = System.currentTimeMillis()
            )
        )
    }

    return targets
}

@Composable
private fun CrosshairOverlay() {
    Canvas(modifier = Modifier.fillMaxSize()) {
        val centerX = size.width / 2
        val centerY = size.height / 2
        val crosshairSize = 40f

        // Main crosshair lines
        drawLine(
            color = BrightGreen,
            start = Offset(centerX - crosshairSize, centerY),
            end = Offset(centerX + crosshairSize, centerY),
            strokeWidth = 2f
        )
        drawLine(
            color = BrightGreen,
            start = Offset(centerX, centerY - crosshairSize),
            end = Offset(centerX, centerY + crosshairSize),
            strokeWidth = 2f
        )

        // Center dot
        drawCircle(
            color = BrightGreen,
            radius = 3f,
            center = Offset(centerX, centerY)
        )

        // Range circles
        for (i in 1..3) {
            drawCircle(
                color = BrightGreen,
                radius = crosshairSize * i,
                center = Offset(centerX, centerY),
                style = Stroke(
                    width = 1f,
                    pathEffect = PathEffect.dashPathEffect(floatArrayOf(5f, 5f))
                )
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
            color = BrightGreen.copy(alpha = 0.5f),
            start = Offset(0f, y),
            end = Offset(size.width, y),
            strokeWidth = 2f
        )

        // Fade effect
        val fadeHeight = 80f
        for (i in 0 until fadeHeight.toInt() step 2) {
            val alpha = (1f - (i / fadeHeight)) * 0.2f
            drawLine(
                color = BrightGreen.copy(alpha = alpha),
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
    onClick: () -> Unit
) {
    val markerColor = when (target.targetType) {
        TargetType.HUMAN -> Color(0xFF6B3A3A) // Dark red
        TargetType.VEHICLE -> Color(0xFF6B5A3A) // Dark amber
        TargetType.STRUCTURE -> Color(0xFF3A5A5A) // Dark cyan
        TargetType.UNKNOWN -> Color(0xFF4A4A4A) // Dark gray
    }

    // Animate pulsing effect
    var pulseAlpha by remember { mutableStateOf(1f) }

    LaunchedEffect(target.id) {
        while (true) {
            pulseAlpha = 0.4f
            delay(600)
            pulseAlpha = 1f
            delay(600)
        }
    }

    BoxWithConstraints(modifier = Modifier.fillMaxSize()) {
        val xPos = maxWidth * target.screenX
        val yPos = maxHeight * target.screenY

        Box(
            modifier = Modifier
                .offset(x = xPos, y = yPos)
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
                    topLeft = Offset(centerX - boxSize / 2, centerY - boxSize / 2),
                    size = androidx.compose.ui.geometry.Size(boxSize, boxSize),
                    style = Stroke(width = 3f)
                )

                // Corner brackets
                val bracketSize = 12f
                val positions = listOf(
                    Offset(centerX - boxSize / 2, centerY - boxSize / 2),
                    Offset(centerX + boxSize / 2 - bracketSize, centerY - boxSize / 2),
                    Offset(centerX - boxSize / 2, centerY + boxSize / 2 - bracketSize),
                    Offset(centerX + boxSize / 2 - bracketSize, centerY + boxSize / 2 - bracketSize)
                )

                positions.forEach { pos ->
                    drawLine(
                        color = markerColor,
                        start = pos,
                        end = Offset(pos.x + bracketSize, pos.y),
                        strokeWidth = 2f
                    )
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
                        color = MilitaryTextPrimary,
                        fontSize = 9.sp,
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
}

@Composable
private fun VideoStatusOverlay(modifier: Modifier = Modifier) {
    Column(
        modifier = modifier
            .padding(16.dp)
            .background(Color.Black.copy(alpha = 0.7f), RoundedCornerShape(4.dp))
            .padding(8.dp)
    ) {
        Row(
            horizontalArrangement = Arrangement.spacedBy(4.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Box(
                modifier = Modifier
                    .size(8.dp)
                    .background(StatusConnected, CircleShape)
            )
            Text(
                text = "LIVE",
                color = StatusConnected,
                fontSize = 10.sp,
                fontFamily = FontFamily.Monospace,
                fontWeight = FontWeight.Bold
            )
        }

        Text(
            text = "1920x1080",
            color = MilitaryTextSecondary,
            fontSize = 8.sp,
            fontFamily = FontFamily.Monospace
        )
    }
}
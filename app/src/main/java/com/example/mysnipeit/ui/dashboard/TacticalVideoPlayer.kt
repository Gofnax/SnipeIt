package com.example.mysnipeit.ui.dashboard

import android.content.Context
import androidx.compose.foundation.Canvas
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.geometry.Size
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
import com.example.mysnipeit.data.models.ShootingSolution
import com.example.mysnipeit.data.models.TargetType
import com.example.mysnipeit.ui.components.TacticalCompass
import com.example.mysnipeit.ui.theme.*
import kotlinx.coroutines.delay

@Composable
fun TacticalVideoPlayer(
    detectedTargets: List<DetectedTarget>,
    shootingSolution: ShootingSolution?,  // ← NEW: Receive shooting solution
    selectedTargetId: String?,            // ← NEW: Know which target is selected
    onTargetClick: (DetectedTarget) -> Unit = {},
    onTargetLockToggle: (String) -> Unit = {},  // ← NEW: Lock/unlock callback
    onTargetSelect: (String) -> Unit = {},      // ← NEW: Select for solution callback
    modifier: Modifier = Modifier
) {
    val context = LocalContext.current
    val exoPlayer = remember { createExoPlayer(context) }

    var scanlinePosition by remember { mutableStateOf(0f) }
    var videoTime by remember { mutableStateOf(0L) }

    // Track locked targets
    var lockedTargets by remember { mutableStateOf(setOf<String>()) }

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
            delay(100)
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
                    useController = false
                    setShowBuffering(PlayerView.SHOW_BUFFERING_NEVER)
                }
            },
            modifier = Modifier.fillMaxSize()
        )

        // Tactical overlays
        Box(modifier = Modifier.fillMaxSize()) {
            // Crosshair
            CrosshairOverlay()

            // Scanning line
            ScanLineOverlay(scanlinePosition)

            // Target markers
            SimulatedTargets(videoTime).forEach { target ->
                val isLocked = lockedTargets.contains(target.id)
                val isSelected = target.id == selectedTargetId

                EnhancedTargetMarker(
                    target = target,
                    isLocked = isLocked,
                    isSelected = isSelected,
                    onLockClick = {
                        if (isLocked) {
                            lockedTargets = lockedTargets - target.id
                            // Deselect if unlocking selected target
                            if (isSelected) {
                                onTargetSelect("")
                            }
                        } else {
                            lockedTargets = lockedTargets + target.id
                        }
                        onTargetLockToggle(target.id)
                    },
                    onTargetClick = {
                        // Only allow selection of locked targets
                        if (isLocked) {
                            onTargetSelect(target.id)
                        }
                    }
                )
            }

            // Video status
            VideoStatusOverlay(
                modifier = Modifier.align(Alignment.TopStart)
            )

            // Scanning/Tracking mode indicator
            Box(
                modifier = Modifier
                    .align(Alignment.TopCenter)
                    .padding(16.dp)
                    .background(
                        color = Color.Black.copy(alpha = 0.7f),
                        shape = RoundedCornerShape(4.dp)
                    )
                    .padding(horizontal = 12.dp, vertical = 6.dp)
            ) {
                val hasLockedTargets = lockedTargets.isNotEmpty()
                val hasSelectedTarget = selectedTargetId != null

                Text(
                    text = when {
                        hasSelectedTarget -> "SOLUTION ACTIVE"
                        hasLockedTargets -> " ${lockedTargets.size}LOCKED"
                        else -> "SCANNING"
                    },
                    color = when {
                        hasSelectedTarget -> Color(0xFFFF6B35)
                        hasLockedTargets -> Color(0xFFFFAA00)
                        else -> Color(0xFF00FF41)
                    },
                    fontSize = 12.sp,
                    fontWeight = FontWeight.Bold,
                    fontFamily = FontFamily.Monospace
                )
            }

            //   3D Tactical Compass (bottom-left)
            if (shootingSolution != null && selectedTargetId != null) {
                Box(
                    modifier = Modifier
                        .align(Alignment.BottomStart)
                        .padding(16.dp)
                ) {
                    TacticalCompass(
                        azimuth = shootingSolution.azimuth,
                        elevation = shootingSolution.elevation,
                        confidence = shootingSolution.confidence
                    )
                }
            }
        }
    }
}

private fun createExoPlayer(context: Context): ExoPlayer {
    return ExoPlayer.Builder(context).build().apply {
        val mediaItem = MediaItem.fromUri("android.resource://${context.packageName}/${R.raw.field_video}")
        setMediaItem(mediaItem)
        repeatMode = Player.REPEAT_MODE_ALL
        playWhenReady = true
        prepare()
    }
}

@Composable
private fun SimulatedTargets(videoTime: Long): List<DetectedTarget> {
    val targets = mutableListOf<DetectedTarget>()

    if (videoTime > 2000) {
        targets.add(
            DetectedTarget(
                id = "T1",
                confidence = 0.85f,
                screenX = 0.3f,
                screenY = 0.4f,
                worldLatitude = 35.093,
                worldLongitude = 32.014,
                distance = 420.0,
                bearing = 245.0,
                targetType = TargetType.HUMAN,
                timestamp = System.currentTimeMillis()
            )
        )
    }

    if (videoTime > 8000) {
        targets.add(
            DetectedTarget(
                id = "T2",
                confidence = 0.72f,
                screenX = 0.7f,
                screenY = 0.5f,
                worldLatitude = 35.094,
                worldLongitude = 32.015,
                distance = 580.0,
                bearing = 260.0,
                targetType = TargetType.UNKNOWN,
                timestamp = System.currentTimeMillis()
            )
        )
    }

    if (videoTime > 15000 && videoTime < 25000) {
        targets.add(
            DetectedTarget(
                id = "T3",
                confidence = 0.91f,
                screenX = 0.5f,
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
private fun EnhancedTargetMarker(
    target: DetectedTarget,
    isLocked: Boolean,
    isSelected: Boolean,  // ← NEW
    onLockClick: () -> Unit,
    onTargetClick: () -> Unit  // ← NEW
) {
    val markerColor = when {
        isSelected -> Color(0xFFFF6B35)     // Orange for selected
        isLocked -> Color(0xFFFFAA00)       // Amber for locked
        target.targetType == TargetType.HUMAN -> Color(0xFF00FF41)  // Green
        else -> Color(0xFF00D9FF)           // Cyan
    }

    var pulseAlpha by remember { mutableStateOf(1f) }

    LaunchedEffect(target.id, isLocked, isSelected) {
        if (!isLocked && !isSelected) {
            while (true) {
                pulseAlpha = 0.4f
                delay(600)
                pulseAlpha = 1f
                delay(600)
            }
        } else {
            pulseAlpha = 1f
        }
    }

    BoxWithConstraints(modifier = Modifier.fillMaxSize()) {
        val xPos = maxWidth * target.screenX
        val yPos = maxHeight * target.screenY

        Box(
            modifier = Modifier
                .offset(x = xPos, y = yPos)
                .clickable { onTargetClick() }  // ← Make target clickable for selection
        ) {
            // Target box
            Canvas(modifier = Modifier.size(80.dp)) {
                val boxSize = when {
                    isSelected -> 65f  // Largest
                    isLocked -> 60f    // Medium
                    else -> 50f        // Small
                }
                val centerX = size.width / 2
                val centerY = size.height / 2
                val strokeWidth = when {
                    isSelected -> 5f   // Thickest
                    isLocked -> 4f     // Medium
                    else -> 3f         // Thin
                }

                // Draw target box
                drawRect(
                    color = markerColor.copy(alpha = pulseAlpha),
                    topLeft = Offset(centerX - boxSize / 2, centerY - boxSize / 2),
                    size = Size(boxSize, boxSize),
                    style = Stroke(width = strokeWidth)
                )

                //  Pulsing glow for selected target
                if (isSelected) {
                    drawRect(
                        color = markerColor.copy(alpha = 0.3f),
                        topLeft = Offset(centerX - boxSize / 2 - 4, centerY - boxSize / 2 - 4),
                        size = Size(boxSize + 8, boxSize + 8),
                        style = Stroke(width = 2f)
                    )
                }

                // Corner brackets
                val bracketSize = 15f
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
                        strokeWidth = strokeWidth
                    )
                    drawLine(
                        color = markerColor,
                        start = pos,
                        end = Offset(pos.x, pos.y + bracketSize),
                        strokeWidth = strokeWidth
                    )
                }

                // Center crosshair
                val crosshairSize = 12f
                drawLine(
                    color = markerColor,
                    start = Offset(centerX - crosshairSize, centerY),
                    end = Offset(centerX + crosshairSize, centerY),
                    strokeWidth = 2f
                )
                drawLine(
                    color = markerColor,
                    start = Offset(centerX, centerY - crosshairSize),
                    end = Offset(centerX, centerY + crosshairSize),
                    strokeWidth = 2f
                )

                // Lock indicator
                if (isLocked) {
                    val lockSize = 12f
                    val lockX = centerX + boxSize / 2 - lockSize - 8f
                    val lockY = centerY - boxSize / 2 + 8f

                    drawRect(
                        color = markerColor,
                        topLeft = Offset(lockX, lockY + lockSize * 0.4f),
                        size = Size(lockSize, lockSize * 0.6f),
                        style = Stroke(width = 2f)
                    )

                    drawArc(
                        color = markerColor,
                        startAngle = 180f,
                        sweepAngle = 180f,
                        useCenter = false,
                        topLeft = Offset(lockX + lockSize * 0.2f, lockY),
                        size = Size(lockSize * 0.6f, lockSize * 0.6f),
                        style = Stroke(width = 2f)
                    )
                }

                //  Selection indicator (star/asterisk for selected)
                if (isSelected) {
                    val starSize = 8f
                    val starX = centerX - boxSize / 2 + 8f
                    val starY = centerY - boxSize / 2 + 8f

                    // Draw asterisk/star
                    drawLine(
                        color = markerColor,
                        start = Offset(starX - starSize, starY),
                        end = Offset(starX + starSize, starY),
                        strokeWidth = 3f
                    )
                    drawLine(
                        color = markerColor,
                        start = Offset(starX, starY - starSize),
                        end = Offset(starX, starY + starSize),
                        strokeWidth = 3f
                    )
                }
            }

            // Target info card
            Card(
                modifier = Modifier
                    .align(Alignment.BottomCenter)
                    .offset(y = 50.dp),
                colors = CardDefaults.cardColors(
                    containerColor = Color.Black.copy(alpha = 0.85f)
                ),
                shape = RoundedCornerShape(6.dp)
            ) {
                Column(
                    modifier = Modifier.padding(8.dp),
                    horizontalAlignment = Alignment.CenterHorizontally
                ) {
                    Text(
                        text = "${target.id} | ${target.targetType.name}",
                        color = markerColor,
                        fontSize = 11.sp,
                        fontFamily = FontFamily.Monospace,
                        fontWeight = FontWeight.Bold
                    )

                    Text(
                        text = "${target.distance.toInt()}m",
                        color = MilitaryTextPrimary,
                        fontSize = 10.sp,
                        fontFamily = FontFamily.Monospace
                    )

                    Text(
                        text = "CONF: ${(target.confidence * 100).toInt()}%",
                        color = if (target.confidence > 0.8f) Color(0xFF00FF41) else Color(0xFFFFAA00),
                        fontSize = 9.sp,
                        fontFamily = FontFamily.Monospace
                    )

                    Spacer(modifier = Modifier.height(6.dp))

                    // Lock/Unlock button
                    Button(
                        onClick = onLockClick,
                        modifier = Modifier
                            .height(28.dp)
                            .widthIn(min = 80.dp),
                        colors = ButtonDefaults.buttonColors(
                            containerColor = if (isLocked) {
                                Color(0xFFFFAA00)
                            } else {
                                Color(0xFF00FF41)
                            }
                        ),
                        shape = RoundedCornerShape(4.dp),
                        contentPadding = PaddingValues(horizontal = 10.dp, vertical = 4.dp)
                    ) {
                        Text(
                            text = if (isLocked) "UNLOCK" else "LOCK",
                            fontSize = 10.sp,
                            fontWeight = FontWeight.Bold,
                            color = Color.Black,
                            fontFamily = FontFamily.Monospace
                        )
                    }

                    //  Selection status
                    if (isLocked) {
                        Spacer(modifier = Modifier.height(4.dp))
                        Text(
                            text = if (isSelected) "★ SELECTED" else "TAP TO SELECT",
                            color = if (isSelected) Color(0xFFFF6B35) else Color(0xFF00FF41).copy(alpha = 0.6f),
                            fontSize = 8.sp,
                            fontFamily = FontFamily.Monospace,
                            fontWeight = if (isSelected) FontWeight.Bold else FontWeight.Normal
                        )
                    }
                }
            }
        }
    }
}

// Keep existing CrosshairOverlay, ScanLineOverlay, and VideoStatusOverlay functions unchanged
@Composable
private fun CrosshairOverlay() {
    Canvas(modifier = Modifier.fillMaxSize()) {
        val centerX = size.width / 2
        val centerY = size.height / 2
        val crosshairSize = 40f
        val color = Color(0xFF00FF41)

        drawLine(
            color = color.copy(alpha = 0.7f),
            start = Offset(centerX - crosshairSize, centerY),
            end = Offset(centerX + crosshairSize, centerY),
            strokeWidth = 2f
        )
        drawLine(
            color = color.copy(alpha = 0.7f),
            start = Offset(centerX, centerY - crosshairSize),
            end = Offset(centerX, centerY + crosshairSize),
            strokeWidth = 2f
        )

        drawCircle(
            color = color,
            radius = 3f,
            center = Offset(centerX, centerY)
        )

        for (i in 1..3) {
            drawCircle(
                color = color.copy(alpha = 0.3f),
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
        val color = Color(0xFF00FF41)

        drawLine(
            color = color.copy(alpha = 0.5f),
            start = Offset(0f, y),
            end = Offset(size.width, y),
            strokeWidth = 2f
        )

        val fadeHeight = 80f
        for (i in 0 until fadeHeight.toInt() step 2) {
            val alpha = (1f - (i / fadeHeight)) * 0.2f
            drawLine(
                color = color.copy(alpha = alpha),
                start = Offset(0f, y + i),
                end = Offset(size.width, y + i),
                strokeWidth = 1f
            )
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
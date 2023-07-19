package com.example.jankdemp

import kotlin.random.Random.Default.nextFloat
import kotlin.random.Random.Default.nextLong

fun simulateJank(
    jankProbability: Double = 0.3,
    extremeJankProbability: Double = 0.02
) {
    val probability = nextFloat()

    if (probability > 1 - jankProbability) {
        val delay = if (probability > 1 - extremeJankProbability) {
            nextLong(500, 700)
        } else {
            nextLong(32, 82)
        }

        try {
            Thread.sleep(delay)
        } catch (e: Exception) {
        }
    }
}
package com.example.crash_optimize

data class JavaCrashConfig(
    val crashType: String,
    val crashMessage: String,
    val crashClass: String,
    val crashMethod: String,
    val crashAndroidVersion: Int = 0
)

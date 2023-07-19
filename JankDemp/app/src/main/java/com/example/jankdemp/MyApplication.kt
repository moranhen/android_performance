package com.example.jankdemp

import android.app.Application
import android.util.Log
import com.example.anr_monitor.AnrTracer
import com.example.jank_optimize.JankMonitor
import com.example.launch_optimize.LaunchOptimize

class MyApplication : Application() {
    override fun onCreate() {
        super.onCreate()
        Log.i("startup_optimize", "application onCreate")
        JankMonitor.init(this)
        AnrTracer.init(this)
        LaunchOptimize.delayGC()
    }
}
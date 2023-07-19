package com.example.anr_monitor


import android.app.Application
import android.os.Build
import androidx.annotation.RequiresApi

object AnrTracer {
    private lateinit var application: Application
    fun init(application: Application) {
        this.application = application
        NativeLib().initAnrMonitor()
    }

    @RequiresApi(Build.VERSION_CODES.M)
    @JvmStatic
    fun onANRDumped() {
        val needReport: Boolean = AnrTracerUtils.isMainThreadBlocked()
        if (needReport) {
            AnrTracerUtils.report()
        } else {
            Thread(
                {
                    AnrTracerUtils.checkErrorStateCycle(application)
                }, "Check-ANR-State-Thread"
            ).start()
        }
    }
}
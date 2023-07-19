package com.example.anr_monitor

class NativeLib {

    /**
     * A native method that is implemented by the 'anr_monitor' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI(): String

    external fun initAnrMonitor(): Unit

    companion object {
        // Used to load the 'anr_monitor' library on application startup.
        init {
            System.loadLibrary("anr_monitor")
        }
    }
}
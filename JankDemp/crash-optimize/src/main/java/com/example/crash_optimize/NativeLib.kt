package com.example.crash_optimize

class NativeLib {

    /**
     * A native method that is implemented by the 'crash_optimize' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI(): String
    external fun openNativeAirBag(signal: Int, soName: String, backtrace: String): Unit
    external fun nativeCrash() :Unit
    external fun nativeCrash1() :Unit
    external fun initWithSignals(signals: IntArray):Unit
    companion object {
        // Used to load the 'crash_optimize' library on application startup.
        init {
            System.loadLibrary("crash_optimize")
        }
    }
}
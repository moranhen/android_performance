package com.example.launch_optimize

import com.bytedance.shadowhook.ShadowHook

class NativeLib {

    /**
     * A native method that is implemented by the 'launch_optimize' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI(): String
    external fun bindCore(threadId: Int, core: Int): Unit
    external fun delayGC() : Unit
    external fun delayGCNew( apiLevel :Int,  manufacturer: String):Int
    external fun requestGC( apiLevel :Int,  manufacturer: String):Int
    external fun suppressGC()
    companion object {
        // Used to load the 'launch_optimize' library on application startup.
        init {
            ShadowHook.init(
                ShadowHook.ConfigBuilder().setMode(ShadowHook.Mode.UNIQUE).setDebuggable(true).build()
            )
            System.loadLibrary("launch_optimize")
        }
    }
}
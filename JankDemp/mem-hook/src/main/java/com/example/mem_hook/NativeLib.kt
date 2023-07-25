package com.example.mem_hook

import com.bytedance.android.bytehook.ByteHook

class NativeLib {
    fun initHook() {
        ByteHook.init()
        hookMemory()
    }
    /**
     * A native method that is implemented by the 'mem_hook' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI(): String
    external fun hookMemory()
    external fun testMalloc(mem: Long)
    external fun dump()
    external fun testFree()
    companion object {
        // Used to load the 'mem_hook' library on application startup.
        init {
            System.loadLibrary("mem_hook")
        }
    }
}
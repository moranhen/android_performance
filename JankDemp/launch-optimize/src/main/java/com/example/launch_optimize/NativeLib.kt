package com.example.launch_optimize

class NativeLib {

    /**
     * A native method that is implemented by the 'launch_optimize' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI(): String
    external fun bindCore(threadId: Int, core: Int): Unit
    external fun delayGC() : Unit
    companion object {
        // Used to load the 'launch_optimize' library on application startup.
        init {
            System.loadLibrary("launch_optimize")
        }
    }
}
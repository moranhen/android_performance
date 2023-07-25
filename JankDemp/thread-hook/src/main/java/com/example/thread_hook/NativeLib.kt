package com.example.thread_hook

class NativeLib {

    /**
     * A native method that is implemented by the 'thread_hook' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI(): String
    external fun hookThread() :Unit
    companion object {
        // Used to load the 'thread_hook' library on application startup.
        init {
            System.loadLibrary("thread_hook")
        }
    }
}
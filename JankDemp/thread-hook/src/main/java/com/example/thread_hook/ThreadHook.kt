package com.example.thread_hook

import com.bytedance.android.bytehook.ByteHook

object ThreadHook {
    fun hookThread(){
        ByteHook.init()
        NativeLib().hookThread()
    }
}
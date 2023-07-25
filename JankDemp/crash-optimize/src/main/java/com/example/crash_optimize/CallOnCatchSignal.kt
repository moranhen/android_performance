package com.example.crash_optimize

import android.content.Context

interface CallOnCatchSignal {
    fun checkIsAnr():Boolean
    fun handleAnr(context: Context,logcat: String)
    fun handleCrash(context: Context,signal: Int,logcat:String)
}

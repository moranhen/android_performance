package com.example.crash_optimize

import android.app.Application
import android.os.Build
import android.os.Looper
import android.util.Log
import androidx.annotation.RequiresApi
import com.example.crash_optimize.utils.Utils

object CrashOptimize {
    const val TAG = "hi_signal"
    private var application: Application? = null

    private var callOnCatchSignal: CallOnCatchSignal? = null
    fun setUpJavaAirBag(configList: List<JavaCrashConfig>) {
        Log.i("StabilityOptimize", "Java 安全气囊已开启")
        val preDefaultExceptionHandler = Thread.getDefaultUncaughtExceptionHandler()
        Thread.setDefaultUncaughtExceptionHandler { thread, exception ->
            handleException(preDefaultExceptionHandler, configList, thread, exception)
            if (thread == Looper.getMainLooper().thread) {
                while (true) {
                    try {
                        Looper.loop()
                    } catch (e: Throwable) {
                        handleException(
                            preDefaultExceptionHandler,
                            configList,
                            Thread.currentThread(),
                            e
                        )
                    }
                }
            }
        }
    }
    fun setUpNativeAirBag(signal: Int, soName: String, backtrace: String) {
        Log.i("StabilityOptimize", "Native 安全气囊已开启")
        NativeLib().openNativeAirBag(signal, soName, backtrace)
    }
    private fun handleException(
        preDefaultExceptionHandler: Thread.UncaughtExceptionHandler,
        configList: List<JavaCrashConfig>,
        thread: Thread,
        exception: Throwable
    ) {
        if (configList.any { isStackTraceMatching(exception, it) }) {
            Log.w("StabilityOptimize", "Java Crash 已捕获")
            Log.w("StabilityOptimize", "FATAL EXCEPTION: ${thread.name}")
            Log.w("StabilityOptimize", exception.message ?: "")
        } else {
            Log.w("StabilityOptimize", "Java Crash 未捕获，交给原有 ExceptionHandler 处理")
            preDefaultExceptionHandler.uncaughtException(thread, exception)
        }
    }

    private fun isStackTraceMatching(exception: Throwable, config: JavaCrashConfig): Boolean {
        val stackTraceElement = exception.stackTrace[0]
        return config.crashType == exception.javaClass.name
                && config.crashMessage == exception.message
                && config.crashClass == stackTraceElement?.className
                && config.crashMethod == stackTraceElement.methodName
                && (config.crashAndroidVersion == 0 || (config.crashAndroidVersion == Build.VERSION.SDK_INT))
    }
    fun nativeCrash1(){
        NativeLib().nativeCrash()
    }
    fun nativeCrash2(){
        NativeLib().nativeCrash1()
    }

    @JvmStatic
    fun signalError() {
        throw SignalException()
    }



    @RequiresApi(Build.VERSION_CODES.M)
    @JvmStatic
    fun callNativeException(signal: Int, nativeStackTrace: String) {
        Log.i(TAG, "callNativeException $signal")
        //Log.i(TAG, getStacktraceForMainThread())

        // 处理anr的场景
        if(signal == SignalConst.SIGQUIT && callOnCatchSignal?.checkIsAnr() == true){
            application?.let {
                callOnCatchSignal?.handleAnr(it, Utils.getLogcat(20,20,20))
            }
            return
        }
        application?.let {
            callOnCatchSignal?.handleCrash(it,signal, Utils.getLogcat(20,20,20))
        }



    }


    @JvmStatic
    fun initSignal(signals: IntArray, application: Application, callOnCatchSignal: CallOnCatchSignal) {
        this.application = application
        this.callOnCatchSignal = callOnCatchSignal
        NativeLib().initWithSignals(signals)
    }


}
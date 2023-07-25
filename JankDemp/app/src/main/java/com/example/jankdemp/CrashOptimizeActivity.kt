package com.example.jankdemp

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.view.View
import com.example.crash_optimize.CrashOptimize
import com.example.crash_optimize.JavaCrashConfig
import com.example.crash_optimize.SignalConst

class CrashOptimizeActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_crash_optimize)
        initView()
    }
    fun initView(){
        findViewById<View>(R.id.crash_java_button).setOnClickListener {
            CrashOptimize.setUpJavaAirBag(
                mutableListOf(
                    JavaCrashConfig(
                        "java.lang.NullPointerException",
                        "test java exception",
                        "com.example.jankdemp.CrashOptimizeActivity",
                        "initView\$lambda\$2"
                    ),
                    JavaCrashConfig(
                        "java.lang.NullPointerException",
                        "test child thread java exception",
                        "com.example.jankdemp.CrashOptimizeActivity",
                        "initView\$lambda\$4\$lambda\$3"
                    )
                )
            )
        }
        findViewById<View>(R.id.crash_native_button).setOnClickListener {
            CrashOptimize.setUpNativeAirBag(
                11,
                "libcrash_optimize.so",
                "Java_com_example_crash_1optimize_NativeLib_nativeCrash"
            )
        }
        findViewById<View>(R.id.crash_native_button1).setOnClickListener {
            CrashOptimize.initSignal(intArrayOf(
                SignalConst.SIGQUIT,
                SignalConst.SIGABRT,
                SignalConst.SIGSEGV),this.application,MyHandler())
        }
        findViewById<View>(R.id.java_crash_button).setOnClickListener {
            throw NullPointerException("test java exception")
        }
        findViewById<View>(R.id.java_crash_button1).setOnClickListener {
            Thread {
                throw NullPointerException("test child thread java exception")
            }.start()
        }
        findViewById<View>(R.id.native_crash_button).setOnClickListener {
            CrashOptimize.nativeCrash1()
        }
        findViewById<View>(R.id.native_crash_button1).setOnClickListener {
            CrashOptimize.nativeCrash2()
        }
    }

}
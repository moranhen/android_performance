package com.example.jankdemp

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.view.View
import com.example.thread_hook.ThreadHook
import kotlin.concurrent.thread

class ThreadActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_thread)
        initView()
    }
    fun initView(){
        findViewById<View>(R.id.open_thread_button).setOnClickListener {
            ThreadHook.hookThread()
        }
        findViewById<View>(R.id.test_thread_button).setOnClickListener {
            testThread()
        }

    }
    private fun testThread() {
        for (i in 0 until 100) {
            thread(start = true) {
                while (true) {
                    Thread.sleep(1000)
                }
            }
        }
    }
}
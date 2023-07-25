package com.example.jankdemp

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.view.View
import com.example.mem_hook.MemHook
import com.example.thread_hook.ThreadHook

class MemoryActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_memory)
        initView()
    }
    fun initView(){
        findViewById<View>(R.id.open_mem_button).setOnClickListener {
            MemHook.hookMemory()
        }
        findViewById<View>(R.id.malloc88_button).setOnClickListener {
            MemHook.testMalloc(88 * 1024 * 1024);
        }
        findViewById<View>(R.id.free_mem_button).setOnClickListener {
            MemHook.testFree()
        }
        findViewById<View>(R.id.dump_mem_button).setOnClickListener {
            MemHook.dump()
        }

    }
}
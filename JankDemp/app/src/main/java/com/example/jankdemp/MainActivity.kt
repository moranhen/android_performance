package com.example.jankdemp

import android.content.Intent
import android.os.Bundle
import android.widget.Button
import androidx.appcompat.app.AppCompatActivity
import com.example.launch_optimize.LaunchOptimize
import java.io.Closeable
import java.lang.ref.WeakReference
import java.util.Arrays

class MainActivity : AppCompatActivity() {
    private var cache = ArrayList<ByteArray>()
    private var weak = WeakReference(WeakClass())
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        initView()
        LaunchOptimize.bindCore()
//        LaunchOptimize.requestGC()
    }
    private fun initView(){
        val button = findViewById<Button>(R.id.jank_button)
        val crashButton = findViewById<Button>(R.id.crash_button)
        val anr = findViewById<Button>(R.id.anr_button)
        val gc = findViewById<Button>(R.id.gc_button)
        val thread = findViewById<Button>(R.id.thread_button)
        val mem = findViewById<Button>(R.id.mem_button)
        val gcNew = findViewById<Button>(R.id.gc_new_button)
        val requestGc = findViewById<Button>(R.id.gc_request_button)
        val alloc = findViewById<Button>(R.id.malloc50_button)
        val createWeak = findViewById<Button>(R.id.create_weak_button)
        val suppressGc = findViewById<Button>(R.id.gc_supress_button)
        button.setOnClickListener {
            val intent = Intent(this,JankActivity::class.java)
            startActivity(intent)
        }
        anr.setOnClickListener {
            try {
                Thread.sleep(20000)
            } catch (e: InterruptedException) {
                e.printStackTrace()
            }
        }
        gc.setOnClickListener {
            try {
                LaunchOptimize.delayGC()
            } catch (e: InterruptedException) {
                e.printStackTrace()
            }
        }
        gcNew.setOnClickListener {
            try {
                LaunchOptimize.delayGCNew()
            } catch (e: InterruptedException) {
                e.printStackTrace()
            }
        }
        requestGc.setOnClickListener {
            try {
                LaunchOptimize.requestGC()
            } catch (e: InterruptedException) {
                e.printStackTrace()
            }
        }
        alloc.setOnClickListener {
            println("123")
            val bytes50 = ByteArray(50 * 1024 * 1024)
            Arrays.fill(bytes50, 0.toByte())
            cache.add(bytes50)
            println("wy 分配50M内存")
        }
        createWeak.setOnClickListener {
            println("Gc weak"+weak.get())
            weak = WeakReference(WeakClass())
        }
        suppressGc.setOnClickListener {
            LaunchOptimize.suppressGc()
        }
        crashButton.setOnClickListener {
            val intent = Intent(this,CrashOptimizeActivity::class.java)
            startActivity(intent)
        }
        thread.setOnClickListener {
            val intent = Intent(this,ThreadActivity::class.java)
            startActivity(intent)
        }
        mem.setOnClickListener {
            val intent = Intent(this,MemoryActivity::class.java)
            startActivity(intent)
        }
    }
    inner class WeakClass : Closeable {
        override fun close() {
            print("GC weak")
        }
    }
}
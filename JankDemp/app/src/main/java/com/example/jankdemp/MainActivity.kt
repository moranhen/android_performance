package com.example.jankdemp

import android.content.Intent
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.widget.Button
import androidx.metrics.performance.PerformanceMetricsState
import androidx.recyclerview.widget.RecyclerView
import com.example.launch_optimize.LaunchOptimize

class MainActivity : AppCompatActivity() {


    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        initView()
        LaunchOptimize.bindCore()
    }
    private fun initView(){
        val button = findViewById<Button>(R.id.jank_button)
        val anr = findViewById<Button>(R.id.anr_button)
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
    }
}
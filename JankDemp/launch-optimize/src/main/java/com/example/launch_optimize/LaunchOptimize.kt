package com.example.launch_optimize

import android.os.Build
import android.os.Process
import java.io.BufferedReader
import java.io.File
import java.io.FileInputStream
import java.io.FileReader
import java.lang.reflect.InvocationTargetException
import java.lang.reflect.Method

object LaunchOptimize {
    fun delayGC() {
//        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.Q) {
            NativeLib().delayGC()
//        }
    }
    fun delayGCNew() {
        println("GC before cm ${Build.VERSION.SDK_INT} ,:: ${Build.MANUFACTURER}")
        NativeLib().delayGCNew(Build.VERSION.SDK_INT, Build.MANUFACTURER)
    }
    fun requestGC() {
        val ret = NativeLib().requestGC(Build.VERSION.SDK_INT, Build.MANUFACTURER)
        if (ret == 0) {
            try {
                val systemClass = System::class.java
                val gcMethod: Method = systemClass.getMethod("gc")
                gcMethod.invoke(null)
            } catch (e: Throwable) {
                e.printStackTrace()
                Runtime.getRuntime().gc()
            }
        }
    }
    fun suppressGc(){
        NativeLib().suppressGC()
    }
    fun bindCore() {
        val threadId = getRenderThreadTid()
        println("123::"+threadId)
        val cpuCores = getNumberOfCPUCores()
        val cpuFreqList = (0 until cpuCores).map {
            Pair(it, getCpuFreq(it))
        }.sortedByDescending {
            it.second
        }
        println("123::"+cpuFreqList[1].first)
        NativeLib().bindCore(threadId, cpuFreqList[1].first)
    }
    private fun getCpuFreq(core: Int): Int {
        val filename = "/sys/devices/system/cpu/cpu$core/cpufreq/cpuinfo_max_freq"
        val cpuInfoMaxFreqFile = File(filename)
        if (cpuInfoMaxFreqFile.exists()) {
            val buffer = ByteArray(128)
            val stream = FileInputStream(cpuInfoMaxFreqFile)
            try {
                stream.read(buffer)
                var endIndex = 0
                //Trim the first number out of the byte buffer.
                while (buffer[endIndex] >= '0'.code.toByte() && buffer[endIndex] <= '9'.code.toByte() && endIndex < buffer.size) endIndex++
                val str = String(buffer, 0, endIndex)
                return str.toInt()
            } catch (e: NumberFormatException) {
                e.printStackTrace()
            } finally {
                stream.close()
            }
        }
        return 0
    }

    private fun getNumberOfCPUCores(): Int {
        return File("/sys/devices/system/cpu/").listFiles { file: File ->
            val path = file.name
            if (path.startsWith("cpu")) {
                for (i in 3 until path.length) {
                    if (path[i] < '0' || path[i] > '9') {
                        return@listFiles false
                    }
                }
                return@listFiles true
            }
            false
        }?.size ?: 0
    }

    private fun getRenderThreadTid(): Int {
        val taskParent = File("/proc/" + Process.myPid() + "/task/")
        if (taskParent.isDirectory) {
            val taskFiles = taskParent.listFiles()
            for (taskFile in taskFiles) {
                //读线程名
                var br: BufferedReader? = null
                var cpuRate = ""
                try {
                    br = BufferedReader(FileReader(taskFile.path + "/stat"), 100)
                    cpuRate = br.readLine()
                } catch (throwable: Throwable) {
                    throwable.printStackTrace()
                } finally {
                    br?.close()
                }
                if (cpuRate.isNotEmpty()) {
                    val param = cpuRate.split(" ".toRegex()).dropLastWhile { it.isEmpty() }
                        .toTypedArray()
                    if (param.size < 2) {
                        continue
                    }
                    val threadName = param[1]
                    //找到name为RenderThread的线程，则返回第0个数据就是 tid
                    if (threadName == "(RenderThread)") {
                        return param[0].toInt()
                    }
                }
            }
        }
        return -1
    }
}
package com.example.jank_optimize

import android.os.Handler
import android.os.HandlerThread
import android.os.Looper
import android.text.TextUtils
import java.text.SimpleDateFormat
import java.util.Locale
import java.util.concurrent.atomic.AtomicBoolean


class StackSampler {
    private val TAG = "StackSampler"
    private val DEFAULT_SAMPLE_INTERVAL = 300
    private val DEFAULT_MAX_ENTRY_COUNT = 100
    private val SEPARATOR = "\r\n"
    private val TIME_FORMATTER: SimpleDateFormat =
        SimpleDateFormat("MM-dd HH:mm:ss.SSS", Locale.CHINESE)

    private val mRunning = AtomicBoolean(false)
    private var mStackThread: HandlerThread = HandlerThread("BlockMonitor")
    private lateinit var mStackHandler: Handler

    private val sStackMap = LinkedHashMap<Long, String>()
    private var mFilterCache: String? = null

    init {
        mStackThread.start()
        mStackHandler = Handler(mStackThread.looper)
    }
    fun startDump(){
        if (mRunning.get()) {
            return;
        }
        mRunning.set(true);
        mStackHandler.removeCallbacks(mRunnable);
        mStackHandler.postDelayed(mRunnable, DEFAULT_SAMPLE_INTERVAL.toLong());
    }
    fun getThreadStackEntries(startTime: Long, endTime: Long): ArrayList<String> {
        val result = ArrayList<String>()
        synchronized(sStackMap) {
            for (entryTime in sStackMap.keys) {
                if (startTime < entryTime && entryTime < endTime) {
                    result.add(
                        TIME_FORMATTER.format(entryTime)
                                + SEPARATOR
                                + SEPARATOR
                                + sStackMap[entryTime]
                    )
                }
            }
        }
        return result
    }
    fun stopDump() {
        if (!mRunning.get()) {
            return
        }
        mRunning.set(false)
        mFilterCache = null
        mStackHandler.removeCallbacks(mRunnable)
    }
    fun shutDown(){
        stopDump()
        mStackThread.quit()
    }
    private val mRunnable = object : Runnable {
        override fun run() {
            dumpInfo()
            if (mRunning.get()) {
                mStackHandler.postDelayed(this, DEFAULT_SAMPLE_INTERVAL.toLong())
            }
        }
    }
    private fun dumpInfo() {
        val stringBuilder = StringBuilder()
        val thread = Looper.getMainLooper().thread
        for (stackTraceElement in thread.stackTrace) {
            stringBuilder
                .append(stackTraceElement.toString())
                .append(SEPARATOR)
        }
        synchronized(sStackMap) {
            if (sStackMap.size == DEFAULT_MAX_ENTRY_COUNT) {
                sStackMap.remove(sStackMap.keys.iterator().next())
            }
            if (!shouldIgnore(stringBuilder)) {
                sStackMap[System.currentTimeMillis()] = stringBuilder.toString()
            }
        }
    }

    /**
     * 过滤掉重复项
     *
     * @param builder
     * @return
     */
    private fun shouldIgnore(builder: java.lang.StringBuilder): Boolean {
        if (TextUtils.equals(mFilterCache, builder.toString())) {
            return true
        }
        mFilterCache = builder.toString()
        return false
    }

}

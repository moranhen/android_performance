package com.example.jank_optimize

import android.os.SystemClock
import android.util.Log
import android.util.Printer


class MonitorCore: Printer {

    private val TAG = "MonitorCore"

    /**
     * 卡顿阈值
     */
    private val BLOCK_THRESHOLD_MILLIS = 200
    private var mStartTime: Long = 0
    private var mStartThreadTime: Long = 0
    private var mPrintingStarted = false
    private var mStackSampler:StackSampler = StackSampler();


    override fun println(x: String?) {
        if (!mPrintingStarted) {
            mStartTime = System.currentTimeMillis()
            mStartThreadTime = SystemClock.currentThreadTimeMillis()
            mPrintingStarted = true
            mStackSampler.startDump()
        } else {
            val endTime = System.currentTimeMillis()
            val endThreadTime = SystemClock.currentThreadTimeMillis()
            mPrintingStarted = false
            if (isBlock(endTime)) {
                val entries: ArrayList<String> =
                    mStackSampler.getThreadStackEntries(mStartTime, endTime)
                if (entries.size > 0) {
                    val blockInfo: BlockInfo = BlockInfo
                        .setMainThreadTimeCost(mStartTime, endTime, mStartThreadTime, endThreadTime)
                        .setThreadStackEntries(entries)
                        .flushString()
                    Log.v(TAG, blockInfo.toString())
                }
            }
            mStackSampler.stopDump()
        }

    }

    fun isBlock(endTime : Long):Boolean{
        return endTime- mStartTime > BLOCK_THRESHOLD_MILLIS
    }
    fun shutDown(){
        mStackSampler.shutDown()
    }
}
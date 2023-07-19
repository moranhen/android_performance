package com.example.jank_optimize

import java.text.SimpleDateFormat
import java.util.Locale


object BlockInfo {
    const val SEPARATOR = "\r\n"
    private const val KV = " = "
    private val TIME_FORMATTER: SimpleDateFormat =
        SimpleDateFormat("MM-dd HH:mm:ss.SSS", Locale.CHINESE)

    const val NEW_INSTANCE_METHOD = "newInstance: "
    private const val KEY_TIME_COST = "time"
    private const val KEY_THREAD_TIME_COST = "thread-time"
    private const val KEY_TIME_COST_START = "time-start"
    private const val KEY_TIME_COST_END = "time-end"
    private const val KEY_STACK = "stack"


    // Per Block Info fields
    var timeCost: Long = 0
    private var threadTimeCost: Long = 0
    var time: Long = 0

    var timeStart: String? = null
    private var timeEnd: String? = null
    var threadStackEntries = ArrayList<String>()

    private val timeSb = StringBuilder()
    private val stackSb = StringBuilder()
    var concernStackString: String? = null
    fun flushString(): BlockInfo {
        val separator = SEPARATOR
        timeSb.append(KEY_TIME_COST).append(KV).append(timeCost).append(separator)
        timeSb.append(KEY_THREAD_TIME_COST).append(KV).append(threadTimeCost).append(separator)
        timeSb.append(KEY_TIME_COST_START).append(KV).append(timeStart).append(separator)
        timeSb.append(KEY_TIME_COST_END).append(KV).append(timeEnd).append(separator)
        if (!threadStackEntries.isEmpty()) {
            val temp = java.lang.StringBuilder()
            for (s in threadStackEntries) {
                temp.append(s)
                temp.append(separator)
            }
            stackSb.append(KEY_STACK).append(KV).append(temp.toString()).append(separator)
        }
        return this
    }


    fun setThreadStackEntries(threadStackEntries: ArrayList<String>): BlockInfo {
        this.threadStackEntries = threadStackEntries
        return this
    }

    fun setMainThreadTimeCost(
        realTimeStart: Long,
        realTimeEnd: Long,
        threadTimeStart: Long,
        threadTimeEnd: Long
    ): BlockInfo {
        timeCost = realTimeEnd - realTimeStart
        threadTimeCost = threadTimeEnd - threadTimeStart
        timeStart = TIME_FORMATTER.format(realTimeStart)
        timeEnd = TIME_FORMATTER.format(realTimeEnd)
        return this
    }

    override fun toString(): String {
        return """
             $timeSb
             $stackSb
             """.trimIndent()
    }
}
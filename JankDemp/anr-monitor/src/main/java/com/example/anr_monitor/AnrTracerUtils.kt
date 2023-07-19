package com.example.anr_monitor

import android.app.ActivityManager
import android.app.ActivityManager.ProcessErrorStateInfo
import android.app.Application
import android.content.Context
import android.os.Build
import android.os.Looper
import android.os.Message
import android.os.Process
import android.os.SystemClock
import android.util.ArrayMap
import android.util.Log
import androidx.annotation.RequiresApi


class AnrTracerUtils {
    companion object {
        private val FOREGROUND_MSG_THRESHOLD: Long = -2000
        private val BACKGROUND_MSG_THRESHOLD: Long = -10000

        private val CHECK_ERROR_STATE_INTERVAL = 500
        private val ANR_DUMP_MAX_TIME = 20000
        private val CHECK_ERROR_STATE_COUNT = ANR_DUMP_MAX_TIME / CHECK_ERROR_STATE_INTERVAL

        @RequiresApi(api = Build.VERSION_CODES.M)
        fun isMainThreadBlocked(): Boolean {
            try {
                val mainQueue = Looper.getMainLooper().queue
                val field = mainQueue.javaClass.getDeclaredField("mMessages")
                field.isAccessible = true
                val mMessage = field.get(mainQueue) as Message?
                if (mMessage != null) {
                    val whenTime: Long = mMessage.getWhen()
                    if (whenTime == 0L) {
                        return false
                    }
                    val time = whenTime - SystemClock.uptimeMillis()
                    var timeThreshold = BACKGROUND_MSG_THRESHOLD
                    val currentForeground: Boolean = isActivityInterestingToUser()
                    if (currentForeground) {
                        timeThreshold = FOREGROUND_MSG_THRESHOLD
                    }
                    return time < timeThreshold
                }
            } catch (e: Exception) {
                return false
            }
            return false
        }

        fun checkErrorStateCycle(application: Application) {
            var checkErrorStateCount = 0
            while (checkErrorStateCount < CHECK_ERROR_STATE_COUNT) {
                try {
                    checkErrorStateCount++
                    val myAnr = checkErrorState(application)
                    if (myAnr) {
                        report()
                        break
                    }
                    Thread.sleep(CHECK_ERROR_STATE_INTERVAL.toLong())
                } catch (t: Throwable) {
                    break
                }
            }
        }

        private fun checkErrorState(application: Application): Boolean {
            try {
                val am = application
                    .getSystemService(Context.ACTIVITY_SERVICE) as ActivityManager
                val procs = am.processesInErrorState ?: return false
                for (proc in procs) {
                    if (proc.uid != Process.myUid()
                        && proc.condition == ProcessErrorStateInfo.NOT_RESPONDING
                    ) {
                        return false
                    }
                    if (proc.pid != Process.myPid()) continue
                    if (proc.condition != ProcessErrorStateInfo.NOT_RESPONDING) {
                        continue
                    }
                    return true
                }
                return false
            } catch (t: Throwable) {
                t.printStackTrace()
            }
            return false
        }

        private fun isActivityInterestingToUser(): Boolean {
            try {
                val activityThreadClass = Class.forName("android.app.ActivityThread")
                val activityThread =
                    activityThreadClass.getMethod("currentActivityThread").invoke(null)
                val activitiesField = activityThreadClass.getDeclaredField("mActivities")
                activitiesField.isAccessible = true
                val activities: Map<Any, Any> =
                    if (Build.VERSION.SDK_INT < Build.VERSION_CODES.KITKAT) {
                        activitiesField[activityThread] as HashMap<Any, Any>
                    } else {
                        activitiesField[activityThread] as ArrayMap<Any, Any>
                    }
                if (activities.isEmpty()) {
                    return false
                }
                for (activityRecord in activities.values) {
                    val activityRecordClass: Class<*> = activityRecord.javaClass
                    val pausedField = activityRecordClass.getDeclaredField("paused")
                    pausedField.isAccessible = true
                    if (!pausedField.getBoolean(activityRecord)) {
                        return true
                    }
                }
            } catch (e: java.lang.Exception) {
                e.printStackTrace()
            }
            return false
        }

        fun getMainThreadJavaStackTrace(): String {
            val stackTrace = StringBuilder()
            for (stackTraceElement in Looper.getMainLooper().thread.stackTrace) {
                stackTrace.append(stackTraceElement.toString() + "\n");
            }
            return stackTrace.toString()
        }

        fun report() {
            Log.e("AnrTracer", "detect Anr");
            Log.e("AnrTracer", getMainThreadJavaStackTrace());
        }
    }

}
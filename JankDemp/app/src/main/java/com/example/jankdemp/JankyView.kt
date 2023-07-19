package com.example.jankdemp

import android.content.Context
import android.graphics.Canvas
import android.util.AttributeSet
import android.view.View

class JankyView @JvmOverloads constructor(
    context: Context,
    attrs: AttributeSet? = null,
    defStyleAttr: Int = 0,
    defStyleRes: Int = 0
) : View(context, attrs, defStyleAttr, defStyleRes) {

    override fun onDraw(canvas: Canvas) {
        simulateJank()
        super.onDraw(canvas)
    }

}
package com.example.mem_hook

object MemHook {
    fun hookMemory(){
        NativeLib().initHook()
    }
    fun testMalloc(mem : Long){
        NativeLib().testMalloc(mem)
    }
    fun testFree(){
        NativeLib().testFree()
    }
    fun dump(){
        NativeLib().dump()
    }
}
//
// Created by wanyan on 2023/7/21.
//
#include <jni.h>
#include <android/log.h>
#include <unistd.h>
#include "art_new.h"
#include "shadowhook.h"
#include "enhanced_dlfcn.h"

#define LOG_TAG            "art-hook"
#define LOGI(fmt, ...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, fmt, ##__VA_ARGS__)
#define LOGE(fmt, ...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, fmt, ##__VA_ARGS__)

// GC
void *originGC = nullptr;
void *gcStub = nullptr;

// JIT
void *originJit = nullptr;
void *jitStub = nullptr;

typedef void (*TaskRunType)(void *, void *);

void newJit(void *task, void *thread) {
    if (jitStub != nullptr) {
        LOGI("gc unhook jit task");
        shadowhook_unhook(jitStub);
    }
    LOGI("gc jit task sleep");
    sleep(5);
    LOGI("gc jit task wake up");
    ((TaskRunType) originJit)(task, thread);
    LOGI("gc jit task done");
}

void newGCDelay(void *task, void *thread) {
    if (gcStub != nullptr) {
        LOGI("unhook gc task");
        shadowhook_unhook(gcStub);
    }
    LOGI("gc task sleep");
    sleep(5);
    LOGI("gc task wake up");
    ((TaskRunType) originGC)(task, thread);
    LOGI("gc task done");
}

void jitDelay() {
    const char *jit = "_ZN3art3jit14JitCompileTask3RunEPNS_6ThreadE";
    jitStub = shadowhook_hook_sym_name("libart.so", jit,
                                       (void *) newJit,
                                       (void **) &originJit);
    if (jitStub != nullptr) {
        LOGI("hook JitCompileTask success");
    } else {
        LOGE("hook JitCompileTask failed");
    }
}

void gcDelay5X() {
    // Android 5.X 没有 ConcurrentGCTask, hook ConcurrentGC
    gcStub = shadowhook_hook_sym_name("libart.so",
                                      "_ZN3art2gc4Heap12ConcurrentGCEPNS_6ThreadE",
                                      (void *) newGCDelay,
                                      (void **) &originGC);
    if (gcStub != nullptr) {
        LOGI("hook ConcurrentGC success");
    } else {
        LOGE("hook ConcurrentGC failed");
    }
}

void gcDelay() {
    // Android 5.X 没有 ConcurrentGCTask, hook ConcurrentGC
    gcStub = shadowhook_hook_sym_name("libart.so",
                                      "_ZN3art2gc4Heap16ConcurrentGCTask3RunEPNS_6ThreadE",
                                      (void *) newGCDelay,
                                      (void **) &originGC);
    if (gcStub != nullptr) {
        LOGI("hook ConcurrentGCTask success");
    } else {
        LOGE("hook ConcurrentGCTask failed");
    }
}


#include <jni.h>
#include <string>
#include <__threading_support>
#include <android/log.h>
#include <dlfcn.h>
#include "enhanced_dlfcn.h"
#include <sys/mman.h>
#include <sched.h>
#include <unistd.h>
#include "xdl.h"
#include "enhanced_dlfcn.h"
#include "art.h"
#include "art_new.h"
#define LOG_TAG            "startup_optimize"
#define LOG(fmt, ...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, fmt, ##__VA_ARGS__)
#define XDL_DEFAULT           0x00
#define XDL_TRY_FORCE_LOAD    0x01
#define XDL_ALWAYS_FORCE_LOAD 0x02
static void **mSlot = nullptr;
static void *originFun = nullptr;
static bool isGcSuppressStart = false;
static int mGcTime = 2; // 默认抑制2s

// 将虚函数表中的值替换成我们hook函数的地址
bool replaceFunc(void **slot, void *func) {
    //将内存页面设置成为可写
    void *page = (void *) ((size_t) slot & (~(size_t) (PAGE_SIZE - 1)));
    if (mprotect(page, PAGE_SIZE, PROT_READ | PROT_WRITE) != 0) return false;
    //将表中的值替换成我们自己的函数
    *slot = func;
#ifdef __arm__
    //刷新内存缓存，使虚函数表修改生效
    cacheflush((long)page, (long)page + PAGE_SIZE, 0);
#endif
    //将内存页面设置成为只读
    mprotect(page, PAGE_SIZE, PROT_READ);
    return true;
}

//我们的 hook 函数
void hookRun(void *thread) {
    //休眠3秒
    LOG("gc hook Run");
    //将虚函数表中的值还原成原函数，避免每次执行run函数时都会执行hook的方法
    replaceFunc(mSlot, originFun);
    ((void (*)(void *)) originFun)(thread);

//    sleep(5);
    LOG("execute origin fun before:: %p", originFun);
    //执行原来的Run方法
    LOG("execute origin fun after::%p", originFun);
    LOG("exec===::%p", mSlot);
    LOG("gc thread addr: %p",thread);
}
void delayGC() {
    //以RTLD_NOW模式打开动态库libart.so，拿到句柄，RTLD_NOW即解析出每个未定义变量的地址
    void *handle = xdl_open("libart.so", XDL_DEFAULT);
    //通过符号拿到ConcurrentGCTask类首地址
    void *taskAddress = xdl_sym(handle, "_ZTVN3art2gc4Heap16ConcurrentGCTaskE",nullptr);
    //通过符号拿到run方法
    void *runAddress = xdl_dsym(handle, "_ZN3art2gc4Heap16ConcurrentGCTask3RunEPNS_6ThreadE",nullptr);
    /*由于 ConcurrentGCTask 只有五个虚函数，所以我们只需要查询前五个地址即可。
    */
    int k = 5;
    for (size_t i = 0; i < k; i++) {
        /*对象头地址中的内容存放的就是是虚函数表的地址，所以这里是指针的指针，即是虚函数表地址
           拿到虚函数表地址后，转换成数组，并遍历获取值
        */
        void *vfunc = ((void **) taskAddress)[i];

        // 如果虚函数表中的值是前面拿到的 Run 函数的地址，那么就找到了Run函数在虚函数表中的地址
        if (vfunc == runAddress) {
            //这里需要注意的是，这里 +i 操作拿到的是地址，而不是值，因为这里的值是 Run 函数的真实地址
            mSlot = (void **) taskAddress + i;
        }
    }
    // 保存原有函数
    originFun = *mSlot;
    // 将虚函数表中的值替换成我们hook函数的地址
    replaceFunc(mSlot, (void *) &hookRun);
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_launch_1optimize_NativeLib_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}
extern "C" JNIEXPORT void JNICALL
Java_com_example_launch_1optimize_NativeLib_bindCore(
        JNIEnv *env,
        jobject /* this */, jint thread_id, jint core) {
    cpu_set_t mask;     //CPU核的集合
    CPU_ZERO(&mask);     //将mask置空
    CPU_SET(core, &mask);    //将需要绑定的cpu核设置给mask，核为序列0,1,2,3……
    if (sched_setaffinity(thread_id, sizeof(mask), &mask) == -1) {     //将线程绑核
        LOG("bind thread %d to core %d fail", thread_id, core);
    } else {
        LOG("bind thread %d to core %d success", thread_id, core);
    }
}
extern "C" JNIEXPORT void JNICALL
Java_com_example_launch_1optimize_NativeLib_delayGC(
        JNIEnv *env,
        jobject /* this */) {
    LOG("Open Startup Optimize");
    delayGC();
}
extern "C" JNIEXPORT int JNICALL
Java_com_example_launch_1optimize_NativeLib_delayGCNew(
        JNIEnv *env,
        jobject /* this */,jint api_level,jstring manufacturer) {
    LOG("GC begin");
    LOG("GC before %d %s",api_level,manufacturer);
    if (api_level < 26 || api_level > 33) {
        LOG("suppressionGC only support android 8-13");
        return -1;
    }
    return suppression_gc(env, api_level, reinterpret_cast<char *>(manufacturer));
}

extern "C" JNIEXPORT int JNICALL
Java_com_example_launch_1optimize_NativeLib_requestGC(
        JNIEnv *env,
        jobject /* this */,jint api_level,jstring manufacturer) {
    LOG("GC end");
    if (api_level < 26 || api_level > 33) {
        LOG("requestGC only support android 8-13");
        return -1;
    }
    return request_gc(env);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_launch_1optimize_NativeLib_suppressGC(JNIEnv *env, jobject thiz) {
    int androidApi = android_get_device_api_level();
    if (androidApi < __ANDROID_API_M__) { // Android 5.x 版本
        gcDelay5X();
    } else if (androidApi < 34) { // Android 6 - 13
        gcDelay();
    } else {
        LOG("android api > 33, return!");
    }


    if (androidApi < __ANDROID_API_N__ || androidApi >= 34) { // Android  < 7  > 13 先不管
        LOG("android api > 33 || < 24, return!");
    } else { // Android 7 - 13
        jitDelay();
    }
}
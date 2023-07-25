#include <jni.h>
#include <string>
#include <android/log.h>
#include <unistd.h>
#include <sys/eventfd.h>
#include <pthread.h>
#define FAIL (-1)
#define LOG_TAG            "native_airbag"
#define LOG(fmt, ...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, fmt, ##__VA_ARGS__)
#define SIGNAL_CRASH_STACK_SIZE (1024 * 128)
#define JNI_CLASS_NAME "com/example/crash_optimize/CrashOptimize"
static struct sigaction old;

JavaVM *javaVm = NULL;
static int notifier = -1;
static jclass callClass;

struct NativeAirBagConfig {
    int signal;
    std::string soName;
    std::string backtrace;
};

static NativeAirBagConfig airBagConfig;

static void sig_handler(int sig, struct siginfo *info, void *ptr) {
    LOG("native Crash happen");
//    auto stackTrace = getStackTraceWhenCrash();
    std::string stackTrace = "123";
    LOG("stack trace %s",stackTrace.c_str());
    if (sig == airBagConfig.signal &&
        stackTrace.find(airBagConfig.soName) != std::string::npos &&
        stackTrace.find(airBagConfig.backtrace) != std::string::npos) {
        LOG("异常信号已捕获");
    } else {
        LOG("异常信号交给原有信号处理器处理");
        sigaction(sig, &old, nullptr);
        raise(sig);
    }
}
void handle_exception1(JNIEnv *env) {
    // 异常处理
    jclass main = env->FindClass( JNI_CLASS_NAME);
    jmethodID id = env->GetStaticMethodID( main, "signalError", "()V");
    env->CallStaticVoidMethod( main, id);
}

//void handle_exception(JNIEnv *env) {
//    LOG("native air bag init failed");
//}

int init_sig_handler(jint signal){
    stack_t ss;
    if (nullptr == (ss.ss_sp = calloc(1, SIGNAL_CRASH_STACK_SIZE))) {
        LOG("native air bag init failed");
        return FAIL;
    }
    ss.ss_size = SIGNAL_CRASH_STACK_SIZE;
    ss.ss_flags = 0;
    if (0 != sigaltstack(&ss, nullptr)) {
        LOG("native air bag init failed");
        return FAIL;
    }
    struct sigaction sigc;
    memset(&sigc, 0, sizeof(sigc));
    sigemptyset(&sigc.sa_mask);
    sigc.sa_sigaction = sig_handler;

    // 推荐采用SA_RESTART 虽然不是所有系统调用都支持，被中断后重新启动，但是能覆盖大部分
    sigc.sa_flags = SA_SIGINFO | SA_ONSTACK | SA_RESTART;
    int flag = sigaction(signal, &sigc, &old);
    if (flag == -1) {
        LOG("native air bag init failed");
        return FAIL;
    }
    LOG("handler init success");
    return flag;
}
static void sig_func(int sig_num, struct siginfo *info, void *ptr) {
    uint64_t data;
    data = sig_num;
    LOG( "catch signal %llu %d", data,notifier);

    if (notifier >= 0) {
        write(notifier, &data, sizeof data);
    }
}

static void* invoke_crash(void *arg){
    JNIEnv *env = NULL;
    if(JNI_OK != javaVm->AttachCurrentThread(&env,arg)){
        return NULL;
    }
    uint64_t data;
    read(notifier,&data,sizeof data);
    jmethodID id = env->GetStaticMethodID(callClass, "callNativeException", "(ILjava/lang/String;)V");
//    jstring nativeStackTrace  = env->NewStringUTF(env,backtraceToLogcat());
    jstring nativeStackTrace = env->NewStringUTF("");
    jint signal_tag = data;
    env->CallStaticVoidMethod(callClass, id,signal_tag,nativeStackTrace);
    env->DeleteLocalRef(nativeStackTrace);


}

void init_with_signal(JNIEnv *env, jintArray signals,void (*handler)(int, struct siginfo *, void *)) {
    // 注意释放内存
    jint *signalsFromJava = env->GetIntArrayElements( signals, 0);
    int size = env->GetArrayLength( signals);
    int needMask = 0;

    for (int i = 0; i < size; i++) {
        if (signalsFromJava[i] == SIGQUIT) {
            needMask = 1;
        }
    }

    do {
        sigset_t mask;
        sigset_t old;
        // 这里需要stack_t，临时的栈，因为SIGSEGV时，当前栈空间已经是处于进程所能控制的栈中，此时就不能在这个栈里面操作，否则就会异常循环
        stack_t ss;
        if (NULL == (ss.ss_sp = calloc(1, SIGNAL_CRASH_STACK_SIZE))) {
//            handle_exception1(env);
            break;
        }
        ss.ss_size = SIGNAL_CRASH_STACK_SIZE;
        ss.ss_flags = 0;
        if (0 != sigaltstack(&ss, NULL)) {
//            handle_exception1(env);
            break;
        }

        if (needMask) {
            sigemptyset(&mask);
            sigaddset(&mask, SIGQUIT);
            if (0 != pthread_sigmask(SIG_UNBLOCK, &mask, &old)) {
                break;
            }
        }


        struct sigaction sigc;
        sigc.sa_sigaction = handler;
        // 信号处理时，先阻塞所有的其他信号，避免干扰正常的信号处理程序
        sigfillset(&sigc.sa_mask);
        sigc.sa_flags = SA_SIGINFO | SA_ONSTACK |SA_RESTART;


        // 注册所有信号
        for (int i = 0; i < size; i++) {
            // 这里不需要旧的处理函数
            // 指定SIGKILL和SIGSTOP以外的所有信号
            int flag = sigaction(signalsFromJava[i], &sigc, NULL);
            if (flag == -1) {
                LOG( "register fail ===== signals[%d] ",i);
//                handle_exception1(env);
                // 失败后需要恢复原样
                if (needMask) {
                    pthread_sigmask(SIG_SETMASK, &old, NULL);
                }
                break;
            }
        }


    } while (0);
    env->ReleaseIntArrayElements( signals, signalsFromJava, 0);

}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env;
    javaVm = vm;
    jclass cls;
    if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK)
        return -1;
    if (NULL == (cls = env->FindClass( JNI_CLASS_NAME))) return -1;

    // 此时的cls仅仅是一个局部变量，如果错误引用会出现错误
    callClass = static_cast<jclass>(env->NewGlobalRef(cls));


    return JNI_VERSION_1_6;
}
extern "C" JNIEXPORT jstring JNICALL
Java_com_example_crash_1optimize_NativeLib_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}
extern "C" JNIEXPORT void JNICALL
Java_com_example_crash_1optimize_NativeLib_openNativeAirBag(
        JNIEnv *env,
        jobject /* this */,
        jint signal,
        jstring soName,
        jstring backtrace) {
    airBagConfig.signal = signal;
    airBagConfig.soName = env->GetStringUTFChars(soName, 0);
    LOG("native air bag:: %s",soName);
    airBagConfig.backtrace = env->GetStringUTFChars(backtrace, 0);
    int flag = init_sig_handler(signal);
}
extern "C" JNIEXPORT void JNICALL
Java_com_example_crash_1optimize_NativeLib_nativeCrash(
        JNIEnv* env,
jobject /* this */) {
    LOG("已保护Native崩溃");
    raise(SIGSEGV);
}
extern "C" JNIEXPORT void JNICALL
Java_com_example_crash_1optimize_NativeLib_nativeCrash1(
        JNIEnv* env,
jobject /* this */) {
    LOG("未保护Native崩溃");
    raise(SIGSEGV);
}


extern "C" JNIEXPORT void JNICALL
Java_com_example_crash_1optimize_NativeLib_initWithSignals(JNIEnv *env, jobject clazz,
                                                           jintArray signals) {
    init_with_signal(env, signals, sig_func);
    notifier = eventfd(0,EFD_CLOEXEC);
    // 启动异步线程
    pthread_t thd;
    if(0 != pthread_create(&thd, NULL,invoke_crash, NULL)){
        handle_exception1(env);
        close(notifier);
        notifier = -1;
    }
}
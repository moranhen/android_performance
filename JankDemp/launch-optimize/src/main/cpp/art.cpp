
#include <android/log.h>
#include "art.h"
#include <cstddef>
#include <unistd.h>
#define LOG_TAG            "gc_suppress"
#define LOG(fmt, ...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, fmt, ##__VA_ARGS__)
void *ArtHelper::runtime_instance_ = nullptr;
unsigned int ArtHelper::api = 0;
JavaVM *ArtHelper::javaVM = nullptr;
void *ArtHelper::heap = nullptr;
int ArtHelper::origin_max_gc_requested_ = 0;
bool ArtHelper::is_gc_suppress_restore = true;
char *ArtHelper::mManufacturer = nullptr;
char meizu[] = "meizu";


void ArtHelper::init(JNIEnv *env, unsigned int apiLevel, char *manufacturer) {
    api = apiLevel;
    LOG("GC api:: %d",api);
    mManufacturer = manufacturer;
    LOG("GC mManufacturer:: %p",*mManufacturer);
    JavaVM *mJavaVm;
    env->GetJavaVM(&mJavaVm);
    javaVM = mJavaVm;
    auto *javaVMExt = (JavaVMExt *) mJavaVm;

    void *runtime = javaVMExt->runtime;
    if (runtime == nullptr) {
        return;
    }
    runtime_instance_ = runtime;
    heap = getHeap();
    LOG("GC get heap %p",heap);
}

//根据art内存偏移获取thread_list，通过javaVM校验
void initOffset() {
    switch (ArtHelper::api) {
#if defined(__arm__)
        case 24: //7.0
        case 25: //7.1
            threadListOffset = 0xe8;
            //4个指针加一个string  http://aospxref.com/android-7.1.2_r39/xref/art/runtime/runtime.h#739
            javaVMThreadListOffset = 4 * 4 + 12;
            break;
        case 26: //8.0
            threadListOffset = 0x114;
            javaVMThreadListOffset = 4 * 4 + 12;
            break;
        case 27: //8.1
            threadListOffset = 0x114;
            javaVMThreadListOffset = 5 * 4 + 12;
            break;
        case 28: //9.0
            threadListOffset = 0x148;
            javaVMThreadListOffset = 5 * 4 + 12;
            break;
        case 29: //10
            threadListOffset = 0x110;
            javaVMThreadListOffset = 4 * 4;
            break;
        case 30: //11
            if (strcmp(meizu, ArtHelper::mManufacturer) == 0) {
                threadListOffset = 0x114;
            } else {
                threadListOffset = 0x10C;
            }
            javaVMThreadListOffset = 5 * 4;
            break;
        case 31: //12
        case 32: //12L
            threadListOffset = 0x118;
            javaVMThreadListOffset = 5 * 4;
            break;
        case 33: //13
            threadListOffset = 0x148;
            javaVMThreadListOffset = 6 * 4;
            break;
#elif defined(__aarch64__)
        case 24: //7.0
        case 25: //7.1
            threadListOffset = 0x180;
            javaVMThreadListOffset = 4 * 8 + 24;
            break;
        case 26: //8.0
            threadListOffset = 0x1C0;
            javaVMThreadListOffset = 4 * 8 + 24;
            break;
        case 27: //8.1
            threadListOffset = 0x1C0;
            javaVMThreadListOffset = 5 * 8 + 24;
            break;
        case 28: //9.0
            threadListOffset = 0x200;
            javaVMThreadListOffset = 5 * 8 + 24;
            break;
        case 29: //10
            threadListOffset = 0x1D0;
            javaVMThreadListOffset = 4 * 8;
            break;
        case 30: //11
            if (strcmp(meizu, ArtHelper::mManufacturer) == 0) {
                threadListOffset = 0x1D0;
            } else {
                threadListOffset = 0x1C8;
            }
            javaVMThreadListOffset = 5 * 8;
            break;
        case 31: //12
        case 32: //12L
            threadListOffset = 0x1e0;
            javaVMThreadListOffset = 5 * 8;
            break;
        case 33: //13
            threadListOffset = 0x240;
            javaVMThreadListOffset = 6 * 8;
            break;
#endif
        default:
            break;
    }
}

//根据art内存偏移获取thread_list, 并根据JavaVm校验是否偏移正确
int checkRuntime(JavaVM *javaVM, void *runtime) {
    if (javaVM == nullptr || runtime == nullptr) {
        return -1;
    }
    initOffset();
    if (threadListOffset <= 0 || javaVMThreadListOffset <= 0) {
        return -1;
    }
    void **thread_list = reinterpret_cast<void **>(reinterpret_cast<intptr_t> (runtime) +
                                                   threadListOffset);
    void **vm = reinterpret_cast<void **>(reinterpret_cast<intptr_t> (thread_list) +
                                          javaVMThreadListOffset);
    if (javaVM == *vm) {
        LOG("runtime check success");
        return 0;
    } else {
        LOG("runtime check failed");
        return -1;
    }
}

void *ArtHelper::getHeap() {
    if (runtime_instance_ == nullptr) {
        return nullptr;
    }
    int offset_heap_in_runtime = -1;
    switch (api) {
#if defined(__arm__)
        case 26: //8.0
        case 27: //8.1
            offset_heap_in_runtime = 0xF4;
            break;
        case 28: //9
            offset_heap_in_runtime = 0x128;
            break;
        case 29: //10
            offset_heap_in_runtime = 0xf0;
            break;
        case 30: //11
            if(strcmp(meizu, mManufacturer) == 0){
                offset_heap_in_runtime = 0xF4;
            } else {
                offset_heap_in_runtime = 0xEC;
            }
            break;
        case 31: //12
        case 32: //12L
            offset_heap_in_runtime = 0xF8;
            break;
        case 33: //13
            offset_heap_in_runtime = 0x128;
            break;
#elif defined(__aarch64__)
        case 26: //8.0
        case 27: //8.1
            offset_heap_in_runtime = 0x180;
            break;
        case 28: //9
            offset_heap_in_runtime = 0x1c0;
            break;
        case 29: //10
            offset_heap_in_runtime = 0x190;
            break;
        case 30: //11
            if (strcmp(meizu, mManufacturer) == 0) {
                offset_heap_in_runtime = 0x190;
            } else {
                offset_heap_in_runtime = 0x188;
            }
            break;
        case 31: //12
        case 32: //12L
            offset_heap_in_runtime = 0x1A0;
            break;
        case 33: //13
            offset_heap_in_runtime = 0x200;
            break;
#endif
        default:
            break;
    }
    if (offset_heap_in_runtime <= 0) {
        return nullptr;
    }
    if (checkRuntime(javaVM, runtime_instance_) == -1) {
        return nullptr;
    }
    void *heap_ = *(void **) ((char *) runtime_instance_ + offset_heap_in_runtime);
    LOG("GC get heap :123");
    return heap_;
}

//根据偏移量获取target_utilization_的值在0-1之间来保证concurrent_gc_pending_和max_gc_requested_获取的是正确的值
void ArtHelper::getConcurrentGcPendingAndMaxGcRequested(bool *&concurrent_gc_pending_,
                                                        int *&max_gc_requested_) {
    switch (api) {
#if defined(__arm__)
        case 26: //8.0
            if (checkTargetUtilization(488)) {
                concurrent_gc_pending_ = (bool *) ((char *) heap + 584);
            }
            break;
        case 27: //8.1
            if (checkTargetUtilization(496)) {
                concurrent_gc_pending_ = (bool *) ((char *) heap + 592);
            }
            break;
        case 28: //9
            if (checkTargetUtilization(464)) {
                concurrent_gc_pending_ = (bool *) ((char *) heap + 560);
            }
            break;
        case 29: //10
            if (checkTargetUtilization(504)) {
                concurrent_gc_pending_ = (bool *) ((char *) heap + 608);
            }
            break;
        case 30: //11
            if (checkTargetUtilization(536)) {
                concurrent_gc_pending_ = (bool *) ((char *) heap + 648);
            } else if(checkTargetUtilization(544)) {
                concurrent_gc_pending_ = (bool *) ((char *) heap + 656); // 三星
            } else if(checkTargetUtilization(552)) {
                concurrent_gc_pending_ = (bool *) ((char *) heap + 664); // OPPO A56
            }
            break;
        case 31: //12
        case 32: //12L
            //不同机型偏移量可能不同，以下四种偏移量可覆盖绝大部分机型
            if (checkTargetUtilization(544)) {
                max_gc_requested_ = (int *) ((char *) heap + 660);
            } else if (checkTargetUtilization(552)) {
                max_gc_requested_ = (int *) ((char *) heap + 668); // 华为
            } else if (checkTargetUtilization(576)) {
                max_gc_requested_ = (int *) ((char *) heap + 708); // 荣耀80
            } else if (checkTargetUtilization(560)) {
                max_gc_requested_ = (int *) ((char *) heap + 676); // 华为 21年之后的机型
            }
            break;
        case 33: //13
            if (checkTargetUtilization(552)) {
                max_gc_requested_ = (int *) ((char *) heap + 668);
            }
            break;
#elif defined(__aarch64__)
        case 26: //8.0
            if (checkTargetUtilization(792)) {
                concurrent_gc_pending_ = (bool *) ((char *) heap + 944);
            }
            break;
        case 27: //8.1
            if (checkTargetUtilization(800)) {
                concurrent_gc_pending_ = (bool *) ((char *) heap + 952);
            } else if (checkTargetUtilization(792)) {
                concurrent_gc_pending_ = (bool *) ((char *) heap + 944);
            }
            break;
        case 28: //9
            if (checkTargetUtilization(752)) {
                concurrent_gc_pending_ = (bool *) ((char *) heap + 904);
            } else if (checkTargetUtilization(760)) {
                concurrent_gc_pending_ = (bool *) ((char *) heap + 912); // oppo大部分机型
            }
            break;
        case 29: //10
            if (checkTargetUtilization(784)) {
                concurrent_gc_pending_ = (bool *) ((char *) heap + 944);
            }
            break;
        case 30: //11
            if (checkTargetUtilization(832)) {
                concurrent_gc_pending_ = (bool *) ((char *) heap + 1000);
            } else if (checkTargetUtilization(840)) {
                concurrent_gc_pending_ = (bool *) ((char *) heap + 1008); // 三星
            } else if (checkTargetUtilization(856)) {
                concurrent_gc_pending_ = (bool *) ((char *) heap + 1024); // OPPO A56
            }
            break;
        case 31: //12
        case 32: //12L
            //不同机型偏移量可能不同，以下四种偏移量可覆盖绝大部分机型
            if (checkTargetUtilization(840)) {
                max_gc_requested_ = (int *) ((char *) heap + 1012);
            } else if (checkTargetUtilization(848)) {
                max_gc_requested_ = (int *) ((char *) heap + 1020); // 华为
            } else if (checkTargetUtilization(880)) {
                max_gc_requested_ = (int *) ((char *) heap + 1076); // 荣耀80
            } else if (checkTargetUtilization(864)) {
                max_gc_requested_ = (int *) ((char *) heap + 1036); // 华为 21年之后的机型
            }
            break;
        case 33: //13
            if (checkTargetUtilization(856)) {
                max_gc_requested_ = (int *) ((char *) heap + 1028);
            }
            break;
#endif
        default:
            break;
    }
}

bool ArtHelper::checkTargetUtilization(int offset) {
    double value = *((double *) ((char *) heap + offset));
    return value > 0.1 && value < 1;
}

int ArtHelper::suppressionGC() {
    if (heap == nullptr) {
        LOG("suppressionGC check failed, heap null");
        return -1;
    }
    bool *concurrent_gc_pending_ = nullptr;
    int *max_gc_requested_ = nullptr;
    // gc抑制新方案说明：https://km.sankuai.com/collabpage/1714434922
    getConcurrentGcPendingAndMaxGcRequested(concurrent_gc_pending_, max_gc_requested_);
    if (api <= 30) { // Android 11及以下
        if (concurrent_gc_pending_ == nullptr) {
            LOG("concurrent_gc_pending_ is null");
            return -1;
        }
        int count = 0;
        while (*concurrent_gc_pending_) {
            // 这里不需要做任何操作，原因是如果concurrent_gc_pending_的值是true说明系统上一次的gc还未完成，当上次gc完成后系统会把该值置为false
            // 我们的目的是抑制下次gc的发生，在进入msc页面之前发生的gc我们无法直接中断，只能等它自行结束
            // 如果超过2s，上次gc仍未结束则跳出循环
            if (count > 40) {
                break;
            }
            usleep(50000); //休眠50ms
            count++;
        }
        *concurrent_gc_pending_ = true;
    } else { // Android 12、13
        if (max_gc_requested_ == nullptr) {
            LOG("max_gc_requested_ is null");
            return -1;
        }
        origin_max_gc_requested_ = *max_gc_requested_;
        *max_gc_requested_ = std::numeric_limits<int>::max();
    }
    is_gc_suppress_restore = false;
    return 0;
}

int ArtHelper::requestGC() {
    if (heap == nullptr) {
        LOG("requestGC check failed, heap null");
        return -1;
    }
    bool *concurrent_gc_pending_ = nullptr;
    int *max_gc_requested_ = nullptr;
    getConcurrentGcPendingAndMaxGcRequested(concurrent_gc_pending_, max_gc_requested_);
    if (api <= 30) { // Android 11及以下
        if (concurrent_gc_pending_ == nullptr) {
            return -1;
        } else {
            *concurrent_gc_pending_ = false;
        }
    } else { // Android 12、13
        if (max_gc_requested_ == nullptr) {
            return -1;
        } else {
            *max_gc_requested_ = origin_max_gc_requested_;
        }
    }
    is_gc_suppress_restore = true;
    return 0;
}

int suppression_gc(JNIEnv *env, unsigned int apiLevel, char *manufacturer) {
    if (!ArtHelper::is_gc_suppress_restore) {
        LOG("last gc suppression not restore");
        return 0;
    }
    LOG("GC 123 %d %p",apiLevel,manufacturer);
    ArtHelper::init(env, apiLevel, manufacturer);
    return ArtHelper::suppressionGC();
}

int request_gc(JNIEnv *env) {
    return ArtHelper::requestGC();
}
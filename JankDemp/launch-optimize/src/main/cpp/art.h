

#ifndef JANKDEMP_ART_H
#define JANKDEMP_ART_H

#include <jni.h>
#include <list>
#include <stdint.h>
#include <string>
#include <vector>
#include <memory>



struct JavaVMExt {
    void *functions;
    void *runtime;
};

class ArtHelper {
public:
    static void init(JNIEnv *, unsigned int, char *);

    static void *getHeap();

    static void getConcurrentGcPendingAndMaxGcRequested(bool *&concurrent_gc_pending_, int *&max_gc_requested_);

    static int suppressionGC();

    static int requestGC();

    static bool checkTargetUtilization(int offset);

    static unsigned int api;

    static char* mManufacturer;

    static int origin_max_gc_requested_;

    static bool is_gc_suppress_restore;

private:
    static void *runtime_instance_;
    static JavaVM *javaVM;
    static void *heap;
};

static uint32_t threadListOffset{0}; //thread_list在runtime中的偏移

static uint32_t javaVMThreadListOffset{0}; //thread_list和javaVM的相应偏移

#ifdef __cplusplus
extern "C" {
#endif
int suppression_gc(JNIEnv *env, unsigned int apiLevel, char *manufacturer);
int request_gc(JNIEnv *env);
#ifdef __cplusplus
};
#endif

#endif //JANKDEMP_ART_H
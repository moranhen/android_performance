#include <unwind.h>
#include <dlfcn.h>
#include <iomanip>
#include <sstream>
#include <android/log.h>
#include <android/log.h>
#define LOG_TAG            "unwind"
#define LOG(fmt, ...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, fmt, ##__VA_ARGS__)
struct BacktraceState {
    void **current;
    void **end;
};


static _Unwind_Reason_Code unwindCallback(struct _Unwind_Context *context, void *args) {
    BacktraceState *state = static_cast<BacktraceState *>(args);
    uintptr_t pc = _Unwind_GetIP(context);
    if (pc) {
        if (state->current == state->end) {
            return _URC_END_OF_STACK;
        } else {
            *state->current++ = reinterpret_cast<void *>(pc);
        }
    }
    return _URC_NO_REASON;
}


size_t captureBacktrace(void **buffer, size_t max) {
    LOG("captureBacktrace begain::");
    BacktraceState state = {buffer, buffer + max};
    LOG("captureBacktrace 1::");

    _Unwind_Backtrace(unwindCallback, &state);


    LOG("captureBacktrace 2::");
    // 获取大小
    LOG("captureBacktrace end::");
    return state.current - buffer;
}


void dumpBacktrace(std::ostream &os, void **buffer, size_t count) {
    LOG("dumpBacktrace begain::");
    for (size_t idx = 0; idx < count; ++idx) {
        const void *addr = buffer[idx];
        const char *symbol = "";
        Dl_info info;
        if (dladdr(addr, &info) && info.dli_sname) {
            symbol = info.dli_sname;
        }
        LOG("dumpBacktrace :: %d",idx);
        os << "  #" << std::setw(2) << idx << "at" << info.dli_fname << ": " << addr << "  "
           << symbol << "\n";
    }
}


std::string getStackTraceWhenCrash() {
    const size_t max = 30;
    void *buffer[max];
    std::ostringstream oss;
    LOG("unwind begain::");
    dumpBacktrace(oss, buffer, captureBacktrace(buffer, max));
    LOG("unwind end::");
    return oss.str();
}




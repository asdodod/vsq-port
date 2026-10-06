#include <android/log.h>
#include <cstdint>
#include <string>

// Paper2 FFI stubs so mod never fails to link or dlopen when paper2 library is absent.
// Routes all log messages directly to Android logcat.

extern "C" {

bool paper2_queue_log_bytes_ffi(int level, const uint8_t *tag_ptr, uintptr_t tag_len, const uint8_t *message_ptr,
                                uintptr_t message_len, const uint8_t *file_ptr, uintptr_t file_len, int line,
                                int column, const uint8_t *function_name_ptr, uintptr_t function_name_len) {
    std::string tag =
        (tag_ptr && tag_len > 0) ? std::string(reinterpret_cast<const char *>(tag_ptr), tag_len) : "VainSabers";
    std::string msg =
        (message_ptr && message_len > 0) ? std::string(reinterpret_cast<const char *>(message_ptr), message_len) : "";

    android_LogPriority prio = ANDROID_LOG_INFO;
    switch (level) {
    case 0:
        prio = ANDROID_LOG_INFO;
        break; // Info
    case 1:
        prio = ANDROID_LOG_WARN;
        break; // Warn
    case 2:
        prio = ANDROID_LOG_ERROR;
        break; // Error
    case 3:
        prio = ANDROID_LOG_DEBUG;
        break; // Debug
    case 4:
        prio = ANDROID_LOG_FATAL;
        break; // Crit
    default:
        prio = ANDROID_LOG_INFO;
        break;
    }
    __android_log_print(prio, tag.c_str(), "%s", msg.c_str());
    return true;
}

bool paper2_wait_for_flush(void) {
    return true;
}

const char *paper2_get_log_directory(void) {
    return "/sdcard/ModData/com.beatgames.beatsaber/Logs";
}
}

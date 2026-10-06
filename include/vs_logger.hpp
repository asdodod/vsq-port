#pragma once
#include <android/log.h>
#include <string>
#include <fmt/core.h>

// A lightweight logger that satisfies beatsaber-hook's is_logger concept
// without requiring paper2_scotland2 at runtime.
struct VsLogger {
    static constexpr const char *TAG = "VainSabers";

    template <typename... Args> void info(fmt::format_string<Args...> fstr, Args &&...args) const {
        auto msg = fmt::format(fstr, std::forward<Args>(args)...);
        __android_log_print(ANDROID_LOG_INFO, TAG, "%s", msg.c_str());
    }

    template <typename... Args> void debug(fmt::format_string<Args...> fstr, Args &&...args) const {
        auto msg = fmt::format(fstr, std::forward<Args>(args)...);
        __android_log_print(ANDROID_LOG_DEBUG, TAG, "%s", msg.c_str());
    }

    template <typename... Args> void warn(fmt::format_string<Args...> fstr, Args &&...args) const {
        auto msg = fmt::format(fstr, std::forward<Args>(args)...);
        __android_log_print(ANDROID_LOG_WARN, TAG, "%s", msg.c_str());
    }

    template <typename... Args> void error(fmt::format_string<Args...> fstr, Args &&...args) const {
        auto msg = fmt::format(fstr, std::forward<Args>(args)...);
        __android_log_print(ANDROID_LOG_ERROR, TAG, "%s", msg.c_str());
    }

    template <typename... Args> void critical(fmt::format_string<Args...> fstr, Args &&...args) const {
        auto msg = fmt::format(fstr, std::forward<Args>(args)...);
        __android_log_print(ANDROID_LOG_FATAL, TAG, "%s", msg.c_str());
    }
};

inline VsLogger vsLogger;

#pragma once

// Include the modloader header
#include "scotland2/shared/modloader.h"

// beatsaber-hook is a modding framework that lets us call functions and fetch
// field values from in the game. It also allows creating objects, configuration,
// and importantly, hooking methods to modify their values.
#include "beatsaber-hook/shared/config/config-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-functions.hpp"
// NOTE: We intentionally do NOT include beatsaber-hook's logging.hpp here
// because it pulls in paper2_scotland2 symbols that may not be present at runtime.

#include "_config.hpp"

// Use android logging directly - avoids paper version conflicts
#include <android/log.h>
#define VS_LOG(fmt, ...) __android_log_print(ANDROID_LOG_INFO, "VainSabers", fmt, ##__VA_ARGS__)

// Our custom logger that satisfies beatsaber-hook's is_logger concept
#include "vs_logger.hpp"

// Define these functions here so that we can easily read configuration and
// log information from other files
Configuration &getConfig();
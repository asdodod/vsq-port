#include "VainSabersUI.hpp"
#include "PresetExport.hpp"
#include "PresetFilePolicy.hpp"
#include "PresetLoader.hpp"
#include "PluginConfig.hpp"

namespace VainSabers {
namespace {
// Notify Android about the finished file so USB/MTP can discover the export.
// A scan failure must not discard a successfully written preset.
bool NotifyExport(const std::filesystem::path &path) {
    if (!modloader_jvm)
        return false;
    JNIEnv *env = nullptr;
    bool attached = false;
    int state = modloader_jvm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6);
    if (state == JNI_EDETACHED) {
        if (modloader_jvm->AttachCurrentThread(&env, nullptr) != JNI_OK)
            return false;
        attached = true;
    } else if (state != JNI_OK)
        return false;
    if (env->PushLocalFrame(16) < 0) {
        env->ExceptionClear();
        if (attached)
            modloader_jvm->DetachCurrentThread();
        return false;
    }
    auto scan = [&]() -> bool {
        auto player = env->FindClass("com/unity3d/player/UnityPlayer");
        if (!player || env->ExceptionCheck())
            return false;
        auto activityField = env->GetStaticFieldID(player, "currentActivity", "Landroid/app/Activity;");
        if (!activityField || env->ExceptionCheck())
            return false;
        auto activity = env->GetStaticObjectField(player, activityField);
        if (!activity || env->ExceptionCheck())
            return false;
        auto scanner = env->FindClass("android/media/MediaScannerConnection");
        if (!scanner || env->ExceptionCheck())
            return false;
        auto method = env->GetStaticMethodID(scanner, "scanFile",
            "(Landroid/content/Context;[Ljava/lang/String;[Ljava/lang/String;Landroid/media/MediaScannerConnection$OnScanCompletedListener;)V");
        if (!method || env->ExceptionCheck())
            return false;
        auto stringClass = env->FindClass("java/lang/String");
        if (!stringClass || env->ExceptionCheck())
            return false;
        auto paths = env->NewObjectArray(1, stringClass, nullptr);
        if (!paths || env->ExceptionCheck())
            return false;
        auto mimeTypes = env->NewObjectArray(1, stringClass, nullptr);
        if (!mimeTypes || env->ExceptionCheck())
            return false;
        auto filename = il2cpp_utils::newcsstr(path.string());
        auto javaPath = env->NewString(reinterpret_cast<const jchar *>(filename->chars), filename->length);
        if (!javaPath || env->ExceptionCheck())
            return false;
        auto mime = env->NewStringUTF("application/octet-stream");
        if (!mime || env->ExceptionCheck())
            return false;
        env->SetObjectArrayElement(paths, 0, javaPath);
        if (env->ExceptionCheck())
            return false;
        env->SetObjectArrayElement(mimeTypes, 0, mime);
        if (env->ExceptionCheck())
            return false;
        env->CallStaticVoidMethod(scanner, method, activity, paths, mimeTypes, nullptr);
        return !env->ExceptionCheck();
    };
    bool requested = scan();
    if (env->ExceptionCheck())
        env->ExceptionClear();
    env->PopLocalFrame(nullptr);
    if (attached)
        modloader_jvm->DetachCurrentThread();
    return requested;
}
std::string ExportOne(const std::string &name, const std::string &source) {
    if (!ValidPresetName(name))
        throw std::runtime_error("Invalid preset filename");
    PresetDocument document;
    if (!document.Parse(source))
        throw std::runtime_error(name + ": unsupported preset JSON");
    const std::filesystem::path directory = PluginConfig::GetPresetDirectory();
    EmbedPresetAssets(document, document.json, directory);
    std::error_code error;
    std::filesystem::create_directories(directory, error);
    if (error)
        throw std::runtime_error("Cannot access /sdcard/VainSabers: " + error.message());
    auto content = document.Serialize();
    Preset preset;
    if (!PresetLoader::LoadFromJsonString(content, preset))
        throw std::runtime_error("Exported preset cannot be parsed");
    const auto path = directory / (name + ".vainsaber");
    WritePresetFile(path, content.data(), content.size());
    std::ifstream file(path, std::ios::binary);
    std::string saved{std::istreambuf_iterator<char>(file), {}};
    if (saved != content)
        throw std::runtime_error("Cannot verify " + path.filename().string());
    VS_LOG("Exported preset with embedded assets: %s (%zu bytes)", path.c_str(), content.size());
    if (!NotifyExport(path))
        VS_LOG("Export saved, but Android file scan could not be requested: %s", path.c_str());
    return "Exported " + path.filename().string();
}
} // namespace
void VainSabersMenuHost::ExportPreset() {
    auto state = State();
    if (!state->editing || IsVainSaberExport(state->document.path.c_str()))
        return;
    try {
        // Like PC, export the open preset's saved name, not an unconfirmed rename.
        state->exportConfirmation = ExportOne(state->document.name, state->document.Serialize());
        state->status.clear();
    } catch (const std::exception &error) {
        state->exportConfirmation = "Export failed";
        state->status = "Export failed: " + std::string(error.what());
        VS_LOG("%s", state->status.c_str());
    }
    BuildEditor();
}
} // namespace VainSabers

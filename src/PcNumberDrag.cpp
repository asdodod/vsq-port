#include "PcNumberDrag.hpp"
#include "PcNumberGesture.hpp"
#include "VRUIControls/VRPointer.hpp"
#include "GlobalNamespace/VRController.hpp"
#include "UnityEngine/Time.hpp"
#include "UnityEngine/Object.hpp"
#include <cmath>
DEFINE_TYPE(VainSabers, PcNumberDrag);
namespace VainSabers {
namespace {
struct DragState {
    PCUI::NumberGesture gesture;
    std::function<float()> value;
    std::function<void(float)> set;
    std::function<void()> input;
    float sensitivity;
};
float Yaw(UnityEngine::Transform *controller) {
    auto f = controller->get_forward();
    return std::atan2(f.x, f.z) * 57.2957795f;
}
} // namespace
void PcNumberDrag::Bind(std::function<float()> value, std::function<void(float)> set, std::function<void()> input,
                        float sensitivity) {
    delete reinterpret_cast<DragState *>(_stateHandle);
    _stateHandle =
        reinterpret_cast<int64_t>(new DragState{{}, std::move(value), std::move(set), std::move(input), sensitivity});
}
void PcNumberDrag::OnPointerDown(UnityEngine::EventSystems::PointerEventData *data) {
    auto s = reinterpret_cast<DragState *>(_stateHandle);
    if (!s)
        return;
    _controller = nullptr;
    UnityW<UnityEngine::Transform> singleController;
    int activePointers = 0;
    for (auto pointer : UnityEngine::Object::FindObjectsOfType<VRUIControls::VRPointer *>()) {
        if (!pointer->get_isActiveAndEnabled())
            continue;
        auto controller = pointer->get_lastSelectedVrController();
        if (!controller)
            continue;
        ++activePointers;
        singleController = controller->get_viewAnchorTransform();
        if (pointer->____currentPointerData == data) {
            _controller = singleController;
            break;
        }
    }
    // Single-pointer modules can wrap event data. Never guess between hands
    // when multiple active pointers are available.
    if (!_controller && activePointers == 1)
        _controller = singleController;
    if (!_controller) {
        VS_LOG("Number drag: no controller for pointer event");
        return;
    }
    s->gesture.Begin(Yaw(_controller), s->value(), UnityEngine::Time::get_unscaledTime());
}
void PcNumberDrag::Update() {
    auto s = reinterpret_cast<DragState *>(_stateHandle);
    if (!s || !s->gesture.pressed || !_controller)
        return;
    float angle;
    if (s->gesture.Move(Yaw(_controller), angle))
        s->set(s->gesture.startValue + angle * s->sensitivity);
}
void PcNumberDrag::OnPointerUp(UnityEngine::EventSystems::PointerEventData *) {
    auto s = reinterpret_cast<DragState *>(_stateHandle);
    _controller = nullptr;
    if (s && s->gesture.Release(UnityEngine::Time::get_unscaledTime()))
        s->input();
}
void PcNumberDrag::OnDisable() {
    _controller = nullptr;
    if (auto s = reinterpret_cast<DragState *>(_stateHandle))
        s->gesture.Cancel();
}
void PcNumberDrag::OnDestroy() {
    delete reinterpret_cast<DragState *>(_stateHandle);
    _stateHandle = 0;
}
} // namespace VainSabers

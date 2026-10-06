#pragma once
#include "main.hpp"
#include "custom-types/shared/macros.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/EventSystems/IPointerDownHandler.hpp"
#include "UnityEngine/EventSystems/IPointerUpHandler.hpp"
#include "UnityEngine/EventSystems/PointerEventData.hpp"
#include <functional>
DECLARE_CLASS_CODEGEN_INTERFACES(VainSabers, PcNumberDrag, UnityEngine::MonoBehaviour,
                                 UnityEngine::EventSystems::IPointerDownHandler *,
                                 UnityEngine::EventSystems::IPointerUpHandler *,
                                 UnityEngine::EventSystems::IEventSystemHandler *) {
    DECLARE_INSTANCE_FIELD(int64_t, _stateHandle);
    DECLARE_INSTANCE_FIELD(UnityW<UnityEngine::Transform>, _controller);
    DECLARE_OVERRIDE_METHOD_MATCH(void, OnPointerDown, &UnityEngine::EventSystems::IPointerDownHandler::OnPointerDown,
                                  UnityEngine::EventSystems::PointerEventData *data);
    DECLARE_OVERRIDE_METHOD_MATCH(void, OnPointerUp, &UnityEngine::EventSystems::IPointerUpHandler::OnPointerUp,
                                  UnityEngine::EventSystems::PointerEventData *data);
    DECLARE_INSTANCE_METHOD(void, Update);
    DECLARE_INSTANCE_METHOD(void, OnDisable);
    DECLARE_INSTANCE_METHOD(void, OnDestroy);

  public:
    void Bind(std::function<float()> value, std::function<void(float)> set, std::function<void()> input,
              float sensitivity);
};

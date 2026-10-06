#pragma once

#include "UnityEngine/Vector2.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Vector4.hpp"
#include "UnityEngine/Quaternion.hpp"
#include "UnityEngine/Matrix4x4.hpp"
#include "UnityEngine/Transform.hpp"

namespace VainSabers {

inline UnityEngine::Vector3 operator+(UnityEngine::Vector3 a, UnityEngine::Vector3 b) {
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}

inline UnityEngine::Vector3 operator-(UnityEngine::Vector3 a, UnityEngine::Vector3 b) {
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

inline UnityEngine::Vector3 operator*(UnityEngine::Vector3 a, float s) {
    return {a.x * s, a.y * s, a.z * s};
}

inline UnityEngine::Vector3 operator*(float s, UnityEngine::Vector3 a) {
    return {a.x * s, a.y * s, a.z * s};
}

inline UnityEngine::Vector3 operator/(UnityEngine::Vector3 a, float s) {
    return {a.x / s, a.y / s, a.z / s};
}

inline UnityEngine::Vector3 operator*(UnityEngine::Quaternion q, UnityEngine::Vector3 v) {
    return UnityEngine::Quaternion::op_Multiply(q, v);
}

struct Pose {
    UnityEngine::Vector3 position;
    UnityEngine::Quaternion rotation;

    constexpr Pose() : position{0.0f, 0.0f, 0.0f}, rotation{0.0f, 0.0f, 0.0f, 1.0f} {}
    constexpr Pose(UnityEngine::Vector3 p, UnityEngine::Quaternion r) : position(p), rotation(r) {}

    inline Pose LerpTo(const Pose &other, float t) const {
        return Pose(UnityEngine::Vector3::Lerp(position, other.position, t),
                    UnityEngine::Quaternion::Slerp(rotation, other.rotation, t));
    }

    inline UnityEngine::Vector3 GetForward() const {
        return UnityEngine::Quaternion::op_Multiply(rotation, UnityEngine::Vector3{0.0f, 0.0f, 1.0f});
    }

    inline UnityEngine::Vector3 GetUp() const {
        return UnityEngine::Quaternion::op_Multiply(rotation, UnityEngine::Vector3{0.0f, 1.0f, 0.0f});
    }

    inline UnityEngine::Vector3 GetRight() const {
        return UnityEngine::Quaternion::op_Multiply(rotation, UnityEngine::Vector3{1.0f, 0.0f, 0.0f});
    }

    inline UnityEngine::Matrix4x4 AsMatrix() const {
        return UnityEngine::Matrix4x4::TRS(position, rotation, UnityEngine::Vector3{1.0f, 1.0f, 1.0f});
    }

    inline Pose TransformPose(const UnityEngine::Matrix4x4 &mat) const {
        UnityEngine::Vector3 pos;
        pos.x = (mat.m00 * position.x) + (mat.m01 * position.y) + (mat.m02 * position.z) + mat.m03;
        pos.y = (mat.m10 * position.x) + (mat.m11 * position.y) + (mat.m12 * position.z) + mat.m13;
        pos.z = (mat.m20 * position.x) + (mat.m21 * position.y) + (mat.m22 * position.z) + mat.m23;

        UnityEngine::Vector3 fwd = GetForward();
        UnityEngine::Vector3 up = GetUp();

        UnityEngine::Vector3 tfwd;
        tfwd.x = (mat.m00 * fwd.x) + (mat.m01 * fwd.y) + (mat.m02 * fwd.z);
        tfwd.y = (mat.m10 * fwd.x) + (mat.m11 * fwd.y) + (mat.m12 * fwd.z);
        tfwd.z = (mat.m20 * fwd.x) + (mat.m21 * fwd.y) + (mat.m22 * fwd.z);

        UnityEngine::Vector3 tup;
        tup.x = (mat.m00 * up.x) + (mat.m01 * up.y) + (mat.m02 * up.z);
        tup.y = (mat.m10 * up.x) + (mat.m11 * up.y) + (mat.m12 * up.z);
        tup.z = (mat.m20 * up.x) + (mat.m21 * up.y) + (mat.m22 * up.z);

        return Pose(pos, UnityEngine::Quaternion::LookRotation(tfwd, tup));
    }

    static inline Pose FromMatrix(const UnityEngine::Matrix4x4 &mat) {
        UnityEngine::Vector3 pos{mat.m03, mat.m13, mat.m23};
        UnityEngine::Vector3 fwd{mat.m02, mat.m12, mat.m22};
        UnityEngine::Vector3 up{mat.m01, mat.m11, mat.m21};
        return Pose(pos, UnityEngine::Quaternion::LookRotation(fwd, up));
    }
};

static inline Pose GetTransformPose(const UnityEngine::Transform *transform) {
    if (!transform)
        return Pose();
    auto *nonConst = const_cast<UnityEngine::Transform *>(transform);
    return Pose(nonConst->get_position(), nonConst->get_rotation());
}

} // namespace VainSabers

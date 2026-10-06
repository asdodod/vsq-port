#pragma once
namespace VainSabers {
// Match PC OBJLoader: v1 imports unchanged, v2 changes right-handed OBJ to Unity.
constexpr float ObjCoordinateZ(float z, int version) {
    return version <= 1 ? z : -z;
}
constexpr bool ReverseObjWinding(int version) {
    return version > 1;
}
} // namespace VainSabers

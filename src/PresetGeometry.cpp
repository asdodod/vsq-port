#include "PresetGeometry.hpp"
#include "PresetResources.hpp"
#include "ObjCoordinates.hpp"
#include "UnityEngine/Bounds.hpp"
#include "UnityEngine/Vector4.hpp"
#include "UnityEngine/Vector2.hpp"
#include <sstream>
#include <algorithm>
#include <cmath>
namespace VainSabers {
namespace {
template <class T> ArrayW<T> Array(const std::vector<T> &v) {
    ArrayW<T> a(v.size());
    std::copy(v.begin(), v.end(), a.begin());
    return a;
}
struct MeshData {
    std::vector<UnityEngine::Vector3> vertices, normals;
    std::vector<UnityEngine::Vector4> tangents, data;
    std::vector<UnityEngine::Vector2> uv;
    std::vector<UnityEngine::Color> colors;
    std::vector<int32_t> triangles;
    int Vertex(UnityEngine::Vector3 position, UnityEngine::Vector3 normal, UnityEngine::Vector2 tex,
               const PartData &part) {
        int index = vertices.size();
        vertices.push_back(position);
        normals.push_back(normal);
        tangents.push_back({0, 0, 0, 0});
        uv.push_back(tex);
        colors.push_back({part.startColor.r, part.startColor.g, part.startColor.b, part.startGlow});
        data.push_back({part.startCustomWeight, part.startOpacity, 0, 0});
        return index;
    }
    UnityEngine::Mesh *Build() {
        if (vertices.empty() || triangles.empty())
            return nullptr;
        auto m = UnityEngine::Mesh::New_ctor();
        m->set_vertices(Array(vertices));
        m->set_normals(Array(normals));
        m->set_tangents(Array(tangents));
        m->set_colors(Array(colors));
        m->set_uv(Array(uv));
        m->SetUVs(1, Array(data));
        m->set_triangles(Array(triangles));
        m->set_bounds({{0, 0, 0}, {100, 100, 100}});
        return m;
    }
};
} // namespace
UnityEngine::Mesh *MakePresetGeometry(const PartData &p) {
    MeshData mesh;
    if (p.geometryMode == GeometryType::Sprite) {
        int nx = std::clamp(p.spriteDivisionsX, 1, 20), ny = std::clamp(p.spriteDivisionsY, 1, 20);
        for (int side = 0; side < (p.doubleSided ? 2 : 1); ++side) {
            int base = mesh.vertices.size();
            float sign = side ? -1 : 1;
            for (int y = 0; y <= ny; ++y)
                for (int x = 0; x <= nx; ++x)
                    mesh.Vertex({(x / (float)nx - .5f) * p.spriteSizeX, (y / (float)ny - .5f) * p.spriteSizeY, 0},
                                {0, 0, sign}, {x / (float)nx, y / (float)ny}, p);
            for (int y = 0; y < ny; ++y)
                for (int x = 0; x < nx; ++x) {
                    int a = base + y * (nx + 1) + x, b = a + 1, c = a + nx + 1, d = c + 1;
                    if (side)
                        mesh.triangles.insert(mesh.triangles.end(), {a, c, b, b, c, d});
                    else
                        mesh.triangles.insert(mesh.triangles.end(), {a, b, c, b, d, c});
                }
        }
        return mesh.Build();
    }
    if (p.geometryMode != GeometryType::Obj)
        return nullptr;
    auto text = DecodePresetAsset(p.objFile, p.objBase64);
    if (text.empty())
        return nullptr;
    std::istringstream file(text);
    std::vector<UnityEngine::Vector3> vertices, normals;
    std::vector<UnityEngine::Vector2> texcoords;
    std::string line;
    try {
        while (std::getline(file, line)) {
            std::istringstream words(line);
            std::string type;
            words >> type;
            if (type == "v" || type == "vn") {
                float x, y, z;
                if (words >> x >> y >> z) {
                    UnityEngine::Vector3 value{x, y, ObjCoordinateZ(z, p.presetVersion)};
                    if (type == "vn") {
                        float magnitude = std::sqrt(x * x + y * y + z * z);
                        if (magnitude > .00001f)
                            value = {value.x / magnitude, value.y / magnitude, value.z / magnitude};
                    }
                    (type == "v" ? vertices : normals).push_back(value);
                }
            } else if (type == "vt") {
                float u, v;
                if (words >> u >> v)
                    texcoords.push_back({u, v});
            } else if (type == "f") {
                std::vector<int> face;
                std::string token;
                while (words >> token) {
                    int ids[3] = {0, 0, 0}, field = 0;
                    std::string value;
                    std::istringstream parts(token);
                    while (field < 3 && std::getline(parts, value, '/')) {
                        if (!value.empty())
                            ids[field] = std::stoi(value);
                        ++field;
                    }
                    auto index = [](int n, size_t size) {
                        return n > 0 ? n - 1 : n < 0 ? static_cast<int>(size) + n : -1;
                    };
                    int vi = index(ids[0], vertices.size()), ti = index(ids[1], texcoords.size()),
                        ni = index(ids[2], normals.size());
                    if (vi < 0 || vi >= vertices.size())
                        throw std::runtime_error("OBJ vertex index");
                    face.push_back(mesh.Vertex(
                        vertices[vi], ni >= 0 && ni < normals.size() ? normals[ni] : UnityEngine::Vector3{0, 0, 0},
                        ti >= 0 && ti < texcoords.size() ? texcoords[ti] : UnityEngine::Vector2{0, 0}, p));
                    if (mesh.vertices.size() > 60000)
                        return nullptr;
                }
                for (size_t i = 1; i + 1 < face.size(); ++i)
                    if (ReverseObjWinding(p.presetVersion))
                        mesh.triangles.insert(mesh.triangles.end(), {face[0], face[i + 1], face[i]});
                    else
                        mesh.triangles.insert(mesh.triangles.end(), {face[0], face[i], face[i + 1]});
            }
        }
        auto result = mesh.Build();
        if (result) {
            if (normals.empty())
                result->RecalculateNormals();
            result->RecalculateBounds();
        }
        return result;
    } catch (...) {
        return nullptr;
    }
}
} // namespace VainSabers

#pragma once
#include <cstdint>
#include <cstring>
namespace draw {
inline uint32_t readWord(const void *p) {
    uint32_t v;
    std::memcpy(&v, p, 4);
    return v;
}
inline void writeWord(void *p, uint32_t v) {
    std::memcpy(p, &v, 4);
}
// Approved RENDER-0005 layout; the semantic renderer never knows offsets.
struct Material {
    uint32_t ambient[4], diffuse[4], specular[4], emission[4], shininess, transparency, textureId,
        unknown;
};
struct Triangle {
    uint32_t normals[3][3], s[3], t[3];
    int32_t vertices[3];
    uint32_t unknown;
};
struct Vertex {
    uint32_t unknown, position[3];
};
struct Mesh {
    int32_t material, membershipCount;
    int32_t *memberships;
};
static_assert(sizeof(Material) == 80 && sizeof(Triangle) == 76 && sizeof(Vertex) == 16 &&
              sizeof(Mesh) == 12);
class ModelView {
    uint8_t *bytes_;
    template <class T> T *table(uint32_t offset) const {
        return reinterpret_cast<T *>(readWord(bytes_ + offset));
    }

  public:
    static constexpr uint32_t size = 0x8d9c4, countsOffset = 0x8d9a4;
    explicit ModelView(void *model) : bytes_(static_cast<uint8_t *>(model)) {
    }
    void *data() const {
        return bytes_;
    }
    int32_t meshCount() const {
        return static_cast<int32_t>(readWord(bytes_ + 0x8d9a4));
    }
    int32_t materialCount() const {
        return static_cast<int32_t>(readWord(bytes_ + 0x8d9ac));
    }
    int32_t triangleCount() const {
        return static_cast<int32_t>(readWord(bytes_ + 0x8d9b4));
    }
    int32_t vertexCount() const {
        return static_cast<int32_t>(readWord(bytes_ + 0x8d9bc));
    }
    Mesh *meshes() const {
        return table<Mesh>(0x8d9a8);
    }
    Material *materials() const {
        return table<Material>(0x8d9b0);
    }
    Triangle *triangles() const {
        return table<Triangle>(0x8d9b8);
    }
    Vertex *vertices() const {
        return table<Vertex>(0x8d9c0);
    }
    Mesh &mesh(int32_t i) const {
        return meshes()[i];
    }
    Material &material(int32_t i) const {
        return materials()[i];
    }
    void setCounts(int32_t meshes, int32_t materials, int32_t triangles, int32_t vertices) const {
        writeWord(bytes_ + 0x8d9a4, meshes);
        writeWord(bytes_ + 0x8d9ac, materials);
        writeWord(bytes_ + 0x8d9b4, triangles);
        writeWord(bytes_ + 0x8d9bc, vertices);
    }
    void setTables(Mesh *m, Material *a, Triangle *t, Vertex *v) const {
        writeWord(bytes_ + 0x8d9a8, reinterpret_cast<uintptr_t>(m));
        writeWord(bytes_ + 0x8d9b0, reinterpret_cast<uintptr_t>(a));
        writeWord(bytes_ + 0x8d9b8, reinterpret_cast<uintptr_t>(t));
        writeWord(bytes_ + 0x8d9c0, reinterpret_cast<uintptr_t>(v));
    }
};
}

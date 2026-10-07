#pragma once
#include "character.hpp"
#include <cstring>
namespace character {
inline uint32_t word(const void *p) {
    uint32_t v;
    std::memcpy(&v, p, 4);
    return v;
}
inline void put(void *p, uint32_t v) { std::memcpy(p, &v, 4); }
inline uint8_t *bytes(void *p) { return static_cast<uint8_t *>(p); }
inline const uint8_t *bytes(const void *p) { return static_cast<const uint8_t *>(p); }
inline void *pointer(const void *p) { return reinterpret_cast<void *>(uintptr_t(word(p))); }
inline Triple triple(const void *p) {
    Triple v;
    std::memcpy(&v, p, sizeof(v));
    return v;
}
// These are approved accessed spans, not allocation or ownership claims.
struct ModelView {
    void *data;
    int32_t meshCount() const { return int32_t(word(data)); }
    int32_t frame(bool b) const { return int32_t(word(bytes(data) + (b ? 52 : 48))); }
    FloatBits factor() const { return word(bytes(data) + 76); }
    void *meshCollection() const { return bytes(data) + 24; }
    void *skinCollection() const { return bytes(data) + 8; }
};
struct MeshView {
    void *data;
    int32_t vertices() const { return int32_t(word(data)); }
    int32_t triangles() const { return int32_t(word(bytes(data) + 4)); }
    int32_t skin() const { return int32_t(word(bytes(data) + 12)); }
    bool textured() const { return bytes(data)[16] != 0; }
    void *positions() const { return pointer(bytes(data) + 272); }
    void *normals() const { return pointer(bytes(data) + 276); }
    void *uvs() const { return pointer(bytes(data) + 280); }
    void *indices() const { return pointer(bytes(data) + 284); }
};
using Gate = uint32_t(__attribute__((thiscall)) *)(void *);
using Lookup = void *(__attribute__((thiscall)) *)(void *, int32_t);
using One = uint32_t(__stdcall *)(uint32_t);
using Two = uint32_t(__stdcall *)(uint32_t, uint32_t);
using Three = uint32_t(__stdcall *)(uint32_t, uint32_t, uint32_t);
using Zero = uint32_t(__stdcall *)();
enum class Api : unsigned { Enable, Bind, Begin, UV, Normal, Vertex, End, Count };
constexpr uint32_t slots[] = {0x18ca68, 0x18ca58, 0x18ca8c, 0x18ca88, 0x18ca84, 0x18ca80, 0x18ca7c};
struct Dependencies {
    Gate gate;
    Lookup mesh, skin;
    uint8_t *module;
    template <class T> T gl(Api api) const {
        return reinterpret_cast<T>(uintptr_t(word(module + slots[unsigned(api)])));
    }
};
class BinarySource final : public Source {
    void *owner_;
    ModelView model_;
    Dependencies callbacks_;

  public:
    BinarySource(void *owner, void *model, Dependencies callbacks)
        : owner_(owner), model_{model}, callbacks_(callbacks) {}
    uint32_t meshGate() override { return callbacks_.gate(model_.meshCollection()); }
    int32_t meshCount() const override { return model_.meshCount(); }
    MeshHandle mesh(int32_t index) override {
        return callbacks_.mesh(model_.meshCollection(), index);
    }
    uint32_t frameOffset(MeshHandle mesh, bool b) const override {
        const uint32_t frame = uint32_t(model_.frame(b));
        const uint32_t vertices = uint32_t(MeshView{mesh}.vertices());
        return frame * vertices; // Contract explicitly specifies low 32-bit product.
    }
    bool textured(MeshHandle mesh) const override { return MeshView{mesh}.textured(); }
    uint32_t texture(MeshHandle mesh) override {
        void *skin = callbacks_.skin(model_.skinCollection(), MeshView{mesh}.skin());
        const uint32_t index = word(bytes(skin) + 516);
        return word(bytes(owner_) + 4 + 4 * index);
    }
    int32_t triangleCount(MeshHandle mesh) const override { return MeshView{mesh}.triangles(); }
    int32_t cornerIndex(MeshHandle mesh, int32_t triangle, unsigned corner) const override {
        return int32_t(
            word(bytes(MeshView{mesh}.indices()) + 24 * uint32_t(triangle) + 4 * corner));
    }
    bool uv(MeshHandle mesh, int32_t index, FloatBits &s, FloatBits &t) const override {
        const MeshView view{mesh};
        void *uv = view.uvs();
        if (!uv)
            return false;
        t = word(bytes(uv) + 8 * uint32_t(index) + 4);
        s = word(bytes(view.uvs()) + 8 * uint32_t(index));
        return true;
    }
    CornerPose pose(MeshHandle mesh, uint32_t a, uint32_t b, int32_t index) const override {
        const MeshView view{mesh};
        CornerPose p;
        p.positionA = triple(bytes(view.positions()) + 12 * (a + uint32_t(index)));
        p.positionB = triple(bytes(view.positions()) + 12 * (b + uint32_t(index)));
        p.normalA = triple(bytes(view.normals()) + 12 * (a + uint32_t(index)));
        p.normalB = triple(bytes(view.normals()) + 12 * (b + uint32_t(index)));
        return p;
    }
    FloatBits factor() const override { return model_.factor(); }
};
class BinaryOutput final : public Output {
    Dependencies callbacks_;

  public:
    explicit BinaryOutput(Dependencies d) : callbacks_(d) {}
    void enableTexture() override { callbacks_.gl<One>(Api::Enable)(0xde1); }
    void bindTexture(uint32_t id) override { callbacks_.gl<Two>(Api::Bind)(0xde1, id); }
    void beginTriangles() override { callbacks_.gl<One>(Api::Begin)(4); }
    void texCoord(FloatBits s, FloatBits t) override { callbacks_.gl<Two>(Api::UV)(s, t); }
    void normal(Triple n) override { callbacks_.gl<Three>(Api::Normal)(n.x, n.y, n.z); }
    void vertex(Triple p) override { callbacks_.gl<Three>(Api::Vertex)(p.x, p.y, p.z); }
    void end() override { callbacks_.gl<Zero>(Api::End)(); }
};
} // namespace character

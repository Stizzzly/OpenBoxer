#include "character.hpp"
#include "character_replay.hpp"
#include "character_trace.hpp"
#include "character_path.hpp"
#include <cstdio>
namespace {
using namespace character;
class MutablePose final : public Source {
  public:
    uint32_t factorBits = 0x3f000000;
    unsigned frameReads = 0;
    bool changed = false, badOffsets = false;
    std::vector<int32_t> cornerOrder;
    uint32_t meshGate() override { return 1; }
    int32_t meshCount() const override { return 1; }
    MeshHandle mesh(int32_t) override { return this; }
    uint32_t frameOffset(MeshHandle, bool frameB) const override {
        ++const_cast<MutablePose *>(this)->frameReads;
        return changed ? 99 : frameB ? 3 : 0;
    }
    bool textured(MeshHandle) const override { return false; }
    uint32_t texture(MeshHandle) override { return 0; }
    int32_t triangleCount(MeshHandle) const override { return 1; }
    int32_t cornerIndex(MeshHandle, int32_t, unsigned corner) const override {
        const_cast<MutablePose *>(this)->cornerOrder.push_back(int32_t(corner));
        return int32_t(corner);
    }
    bool uv(MeshHandle, int32_t, FloatBits &s, FloatBits &t) const override {
        s = 0;
        t = 0x3f800000;
        return true;
    }
    CornerPose pose(MeshHandle, uint32_t a, uint32_t b, int32_t) const override {
        if (a != 0 || b != 3)
            const_cast<MutablePose *>(this)->badOffsets = true;
        const uint32_t positionA = changed ? 0x42c60000 : 0x3f800000; // 99 or 1
        const uint32_t positionB = changed ? 0x4479c000 : 0x40400000; // 999 or 3
        return {{positionA, positionA, positionA},
                {positionB, positionB, positionB},
                {0, 0, 0},
                {0x3f800000, 0x3f800000, 0x3f800000}};
    }
    FloatBits factor() const override { return factorBits; }
};
class MutatingOutput final : public Output {
    MutablePose &source;

  public:
    std::vector<Triple> normals, vertices;
    std::vector<unsigned> events;
    explicit MutatingOutput(MutablePose &s) : source(s) {}
    void enableTexture() override { events.push_back(99); }
    void bindTexture(uint32_t) override { events.push_back(98); }
    void beginTriangles() override { events.push_back(1); }
    void texCoord(FloatBits, FloatBits) override { events.push_back(2); }
    void normal(Triple n) override {
        events.push_back(3);
        normals.push_back(n);
        source.factorBits = 0x3f800000;
        source.changed = true;
    }
    void vertex(Triple p) override {
        events.push_back(4);
        vertices.push_back(p);
    }
    void end() override { events.push_back(5); }
};
unsigned semanticChecks() {
    FpPreserver preserve;
    uint16_t cw = 0x027f;
    __asm__ volatile("fldcw %0" ::"m"(cw));
    unsigned failures = 0;
    if (interpolate(0x3e1f0cda, 0x3efa3f5d, 0x3e513741) != 0x3e64cfc5)
        ++failures;
    if (interpolate(0x40ca2ab1, 0x3e01bee8, 0x3f8db2ac) != 0xbf092bee)
        ++failures;
    if (interpolate(0x40b15ec8, 0x3f3a75c5, 0x3fc5bd66) != 0xbff28799)
        ++failures;
    if (interpolate(0x80000000, 0x80000000, 0xbf800000) != 0x80000000)
        ++failures;
    std::string normalized;
    for (const char *path : {".\\base\\fighters\\\\1_lower.bhm", "BASE/fighters/1_lower.bhm",
                             ".//base//fighters///1_lower.bhm"})
        if (!normalizeAssetPath(path, normalized) || normalized != "base\\fighters\\1_lower.bhm")
            ++failures;
    for (const char *path : {"base\\fighters\\..\\1_lower.bhm", "..\\base\\fighters\\1_lower.bhm",
                             "base\\.\\fighters\\1_lower.bhm", ""})
        if (normalizeAssetPath(path, normalized))
            ++failures;
    if (!normalizeAssetPath("other\\1_lower.bhm", normalized) ||
        normalized == "base\\fighters\\1_lower.bhm")
        ++failures;
    MutablePose source;
    MutatingOutput output(source);
    if (render(source, output) != 1 || source.frameReads != 2 || source.badOffsets ||
        source.cornerOrder != std::vector<int32_t>{2, 1, 0} ||
        output.events != std::vector<unsigned>{1, 2, 3, 4, 2, 3, 4, 2, 3, 4, 5})
        ++failures;
    if (output.vertices.size() != 3 || output.normals.size() != 3)
        ++failures;
    else {
        for (unsigned i = 0; i < 3; ++i) {
            const auto normal = output.normals[i], position = output.vertices[i];
            const uint32_t expectedNormal = i ? 0x3f800000 : 0x3f000000,
                           expectedPosition = i ? 0x4479c000 : 0x40400000;
            if (normal.x != expectedNormal || normal.y != expectedNormal ||
                normal.z != expectedNormal || position.x != expectedPosition ||
                position.y != expectedPosition || position.z != expectedPosition)
                ++failures;
        }
    }
    return failures;
}
} // namespace
int main(int argc, char **argv) {
    const auto failures = argc == 3 ? character::offlineReplay(argv[1], argv[2])
                                    : character::offlineFixtures() + semanticChecks();
    std::printf("RENDER-0006 offline failures=%u\n", failures);
    return failures ? 1 : 0;
}

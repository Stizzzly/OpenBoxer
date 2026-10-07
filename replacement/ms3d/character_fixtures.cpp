#include "character_trace.hpp"
#include "world_runtime.hpp"
#include <array>
#include <cstring>
#include <windows.h>
#include <cmath>
namespace character {
namespace {
constexpr unsigned caseCount = 27;
const char *caseNames[] = {"zero-gate",
                           "zero-mesh-count",
                           "negative-mesh-count",
                           "untextured",
                           "textured",
                           "zero-triangles",
                           "negative-triangles",
                           "null-uv",
                           "factor-zero",
                           "factor-one",
                           "intermediate",
                           "extrapolation",
                           "signed-zero",
                           "multiple-meshes",
                           "normal-factor-change",
                           "uv-geometry-change",
                           "enable-skin-change",
                           "lookup-live-count",
                           "begin-triangle-change",
                           "end-mesh-count",
                           "vertex-triangle-pointer-change",
                           "cached-frame-offsets",
                           "normal-position-pointer-change",
                           "negative-gate",
                           "final-only-f32-rounding-a",
                           "final-only-f32-rounding-b",
                           "negative-factor-signed-zero"};
uint32_t bits(float v) {
    uint32_t result;
    std::memcpy(&result, &v, 4);
    return result;
}
struct Fixture {
    unsigned id = 0;
    uint32_t gate = 1, normalCalls = 0, uvCalls = 0, vertexCalls = 0;
    std::array<uint8_t, 80> model{};
    std::array<uint32_t, 4> owner{};
    std::array<std::array<uint8_t, 288>, 2> meshes{};
    std::array<std::array<uint8_t, 536>, 2> skins{};
    std::array<std::array<Triple, 9>, 2> positions{}, normals{}, alternatePositions{},
        alternateNormals{};
    std::array<std::array<std::array<uint32_t, 2>, 3>, 2> uv{};
    std::array<std::array<std::array<int32_t, 6>, 2>, 2> triangles{}, alternateTriangles{};
    Trace trace;
    void reset(unsigned test) {
        *this = Fixture{};
        id = test;
        put(model.data(), test == 13 || test == 17 ? 2 : 1);
        put(model.data() + 48, 0);
        put(model.data() + 52, 2);
        put(model.data() + 76, bits(.375f));
        owner = {0x51515151, 111, 222, 333};
        for (unsigned m = 0; m < 2; ++m) {
            auto *mesh = meshes[m].data();
            put(mesh, 3);
            put(mesh + 4, 1);
            put(mesh + 12, 0);
            mesh[16] = test == 4 || test == 16;
            put(mesh + 272, uint32_t(uintptr_t(positions[m].data())));
            put(mesh + 276, uint32_t(uintptr_t(normals[m].data())));
            put(mesh + 280, uint32_t(uintptr_t(uv[m].data())));
            put(mesh + 284, uint32_t(uintptr_t(triangles[m].data())));
            put(skins[m].data() + 516, m);
            for (unsigned v = 0; v < 9; ++v) {
                const float value = float(1 + m * 17 + v);
                positions[m][v] = {bits(value), bits(value * .25f), bits(-value * .5f)};
                normals[m][v] = {bits(-value * .125f), bits(value * .375f), bits(value * .0625f)};
                alternatePositions[m][v] = {bits(value + 23), bits(-value - 5), bits(value + 7)};
                alternateNormals[m][v] = {bits(value + .03125f), bits(value + .0625f),
                                          bits(value + .125f)};
            }
            for (unsigned v = 0; v < 3; ++v)
                uv[m][v] = {bits(float(v) * .125f), bits(float(v) * .25f)};
            triangles[m][0] = {0, 1, 2, 0x1234, 0x2345, 0x3456};
            triangles[m][1] = {2, 0, 1, 4, 5, 6};
            alternateTriangles[m][0] = {1, 2, 0, 7, 8, 9};
            alternateTriangles[m][1] = {0, 2, 1, 10, 11, 12};
        }
        put(model.data() + 12, uint32_t(uintptr_t(skins.data())));
        put(model.data() + 28, uint32_t(uintptr_t(meshes.data())));
        put(model.data() + 32, uint32_t(uintptr_t(meshes.data() + 1)));
        if (test == 0)
            gate = 0;
        if (test == 1)
            put(model.data(), 0);
        if (test == 2)
            put(model.data(), uint32_t(-3));
        if (test == 5)
            put(meshes[0].data() + 4, 0);
        if (test == 6)
            put(meshes[0].data() + 4, uint32_t(-1));
        if (test == 7)
            put(meshes[0].data() + 280, 0);
        if (test == 8)
            put(model.data() + 76, bits(0));
        if (test == 9)
            put(model.data() + 76, bits(1));
        if (test == 11)
            put(model.data() + 76, bits(1.75f));
        if (test == 12) {
            for (auto &p : positions[0])
                p = {0x80000000, 0, 0x80000000};
            for (auto &n : normals[0])
                n = {0, 0x80000000, 0};
        }
        if (test == 18)
            put(meshes[0].data() + 4, 0);
        if (test == 23) {
            gate = 0xffffffff;
            put(model.data(), 0);
        }
        if (test == 24 || test == 25) {
            const uint32_t a = test == 24 ? 0x3e1f0cda : 0x40ca2ab1,
                           b = test == 24 ? 0x3efa3f5d : 0x3e01bee8,
                           f = test == 24 ? 0x3e513741 : 0x3f8db2ac;
            put(model.data() + 76, f);
            for (unsigned v = 0; v < 3; ++v) {
                positions[0][v] = {a, a, a};
                positions[0][v + 6] = {b, b, b};
                normals[0][v] = {a, a, a};
                normals[0][v + 6] = {b, b, b};
            }
        }
        if (test == 26) {
            put(model.data() + 76, 0xbf800000);
            for (auto &p : positions[0])
                p = {0x80000000, 0x80000000, 0x80000000};
            for (auto &n : normals[0])
                n = {0x80000000, 0x80000000, 0x80000000};
        }
    }
    std::string meshRole(void *p) const {
        for (unsigned m = 0; m < 2; ++m)
            if (p == meshes[m].data())
                return "mesh" + std::to_string(m);
        return "UNKNOWN";
    }
    std::vector<uint8_t> witness() const {
        std::vector<uint8_t> result;
        const auto add = [&](const void *p, size_t n) {
            const auto *b = static_cast<const uint8_t *>(p);
            result.insert(result.end(), b, b + n);
        };
        add(model.data(), model.size());
        add(owner.data(), sizeof(owner));
        add(meshes.data(), sizeof(meshes));
        add(skins.data(), sizeof(skins));
        add(positions.data(), sizeof(positions));
        add(normals.data(), sizeof(normals));
        add(alternatePositions.data(), sizeof(alternatePositions));
        add(alternateNormals.data(), sizeof(alternateNormals));
        add(uv.data(), sizeof(uv));
        add(triangles.data(), sizeof(triangles));
        add(alternateTriangles.data(), sizeof(alternateTriangles));
        return result;
    }
    void snapshot(FILE *f) const {
        std::fprintf(f,
                     "{\"model\":{\"mesh_count\":%d,\"frame_a\":%d,\"frame_b\":%d,\"factor_bits\":%"
                     "u},\"owner_texture_ids\":[%u,%u,%u],\"meshes\":[",
                     int32_t(word(model.data())), int32_t(word(model.data() + 48)),
                     int32_t(word(model.data() + 52)), word(model.data() + 76), owner[1], owner[2],
                     owner[3]);
        for (unsigned m = 0; m < 2; ++m) {
            const MeshView view{const_cast<uint8_t *>(meshes[m].data())};
            std::fprintf(f,
                         "%s{\"role\":\"mesh%u\",\"vertices_per_frame\":%d,\"triangle_count\":%d,"
                         "\"skin_index\":%d,\"textured\":%s,\"frame_bounds\":3,\"skin_texture_"
                         "index\":%u,\"positions\":[",
                         m ? "," : "", m, view.vertices(), view.triangles(), view.skin(),
                         view.textured() ? "true" : "false", word(skins[m].data() + 516));
            const auto triples = [&](const std::array<Triple, 9> &v) {
                for (unsigned i = 0; i < 9; ++i)
                    std::fprintf(f, "%s[%u,%u,%u]", i ? "," : "", v[i].x, v[i].y, v[i].z);
            };
            triples(positions[m]);
            std::fputs("],\"normals\":[", f);
            triples(normals[m]);
            std::fputs("],\"alternate_positions\":[", f);
            triples(alternatePositions[m]);
            std::fputs("],\"alternate_normals\":[", f);
            triples(alternateNormals[m]);
            std::fputs("],\"uv\":[", f);
            for (unsigned v = 0; v < 3; ++v)
                std::fprintf(f, "%s[%u,%u]", v ? "," : "", uv[m][v][0], uv[m][v][1]);
            std::fputs("],\"triangles\":[", f);
            for (unsigned t = 0; t < 2; ++t) {
                std::fprintf(f, "%s[", t ? "," : "");
                for (unsigned j = 0; j < 6; ++j)
                    std::fprintf(f, "%s%d", j ? "," : "", triangles[m][t][j]);
                std::fputs("]", f);
            }
            std::fputs("]}", f);
        }
        std::fputs("]}", f);
    }
};
thread_local Fixture *current = nullptr;
uint32_t __attribute__((thiscall)) gateCallback(void *collection) {
    FpPreserver preserve;
    current->trace.append("mesh_gate", nullptr, 0,
                          collection == current->model.data() + 24 ? "model.mesh_collection"
                                                                   : "UNKNOWN",
                          current->gate);
    return current->gate;
}
void *__attribute__((thiscall)) meshCallback(void *collection, int32_t index) {
    FpPreserver preserve;
    void *result = current->meshes[unsigned(index)].data();
    uint32_t arg = uint32_t(index);
    current->trace.append("mesh_lookup", &arg, 1,
                          collection == current->model.data() + 24 ? "model.mesh_collection"
                                                                   : "UNKNOWN",
                          0, current->meshRole(result));
    if (current->id == 17)
        put(current->model.data(), 1);
    return result;
}
void *__attribute__((thiscall)) skinCallback(void *collection, int32_t index) {
    FpPreserver preserve;
    uint32_t arg = uint32_t(index);
    current->trace.append("skin_lookup", &arg, 1,
                          collection == current->model.data() + 8 ? "model.skin_collection"
                                                                  : "UNKNOWN",
                          0, "skin" + std::to_string(index));
    return current->skins[unsigned(index)].data();
}
uint32_t __stdcall enableCallback(uint32_t a) {
    current->trace.append("glEnable", &a, 1);
    if (current->id == 16)
        put(current->meshes[0].data() + 12, 1);
    return 0xe1;
}
uint32_t __stdcall bindCallback(uint32_t a, uint32_t b) {
    uint32_t args[] = {a, b};
    current->trace.append("glBindTexture", args, 2);
    return 0xb1;
}
uint32_t __stdcall beginCallback(uint32_t a) {
    current->trace.append("glBegin", &a, 1);
    if (current->id == 18)
        put(current->meshes[0].data() + 4, 1);
    return 0xbe;
}
uint32_t __stdcall uvCallback(uint32_t s, uint32_t t) {
    uint32_t args[] = {s, t};
    current->trace.append("glTexCoord2f", args, 2);
    ++current->uvCalls;
    if (current->id == 15) {
        put(current->meshes[0].data() + 272,
            uint32_t(uintptr_t(current->alternatePositions[0].data())));
        put(current->meshes[0].data() + 276,
            uint32_t(uintptr_t(current->alternateNormals[0].data())));
    }
    if (current->id == 21) {
        put(current->model.data() + 48, 1);
        put(current->model.data() + 52, 1);
        put(current->meshes[0].data(), 2);
    }
    return 0x75;
}
uint32_t __stdcall normalCallback(uint32_t x, uint32_t y, uint32_t z) {
    uint32_t args[] = {x, y, z};
    current->trace.append("glNormal3f", args, 3);
    ++current->normalCalls;
    if (current->id == 14)
        put(current->model.data() + 76, bits(.75f));
    if (current->id == 22) {
        put(current->meshes[0].data() + 272,
            uint32_t(uintptr_t(current->alternatePositions[0].data())));
        put(current->model.data() + 76, bits(.625f));
    }
    return 0x6e;
}
uint32_t __stdcall vertexCallback(uint32_t x, uint32_t y, uint32_t z) {
    uint32_t args[] = {x, y, z};
    current->trace.append("glVertex3f", args, 3);
    ++current->vertexCalls;
    if (current->id == 20)
        put(current->meshes[0].data() + 284,
            uint32_t(uintptr_t(current->alternateTriangles[0].data())));
    return 0x76;
}
uint32_t __stdcall endCallback() {
    current->trace.append("glEnd", nullptr, 0);
    if (current->id == 19)
        put(current->model.data(), 0);
    return 0xed;
}
const uintptr_t glCallbacks[] = {uintptr_t(&enableCallback), uintptr_t(&bindCallback),
                                 uintptr_t(&beginCallback),  uintptr_t(&uvCallback),
                                 uintptr_t(&normalCallback), uintptr_t(&vertexCallback),
                                 uintptr_t(&endCallback)};
Dependencies fixtureDependencies(uint8_t *module) {
    return {gateCallback, meshCallback, skinCallback, module};
}
uint8_t *currentModule = nullptr;
uint32_t __attribute__((thiscall)) candidate(void *owner, void *model) {
    Dependencies d = fixtureDependencies(currentModule);
    BinarySource source(owner, model, d);
    BinaryOutput output(d);
    return render(source, output);
}
bool abiValid(const AbiReport &a) {
    return a.beforeStack == a.afterStack && a.ebx == 0x11223344 && a.esi == 0x22334455 &&
           a.edi == 0x33445566 && a.ebp == 0x44556677 && a.beforeCW == a.afterCW &&
           (a.beforeMX & 0xffc0) == (a.afterMX & 0xffc0);
}
bool writeSlot(void *p, uint32_t v) {
    DWORD old, unused;
    if (!VirtualProtect(p, 4, PAGE_READWRITE, &old))
        return false;
    put(p, v);
    return VirtualProtect(p, 4, old, &unused) != 0;
}
uint32_t run(uintptr_t base, bool differential) {
    FpPreserver preserve;
    uint16_t cw = 0x027f;
    __asm__ volatile("fldcw %0" ::"m"(cw));
    std::vector<uint8_t> moduleStorage;
    if (!differential)
        moduleStorage.resize(0x19f000);
    uint8_t *image = differential ? reinterpret_cast<uint8_t *>(base) : moduleStorage.data();
    currentModule = image;
    uint32_t savedSlots[7];
    for (unsigned i = 0; i < 7; ++i) {
        savedSlots[i] = word(image + slots[i]);
        if (differential) {
            if (!writeSlot(image + slots[i], uint32_t(glCallbacks[i])))
                return 100;
        } else
            put(image + slots[i], uint32_t(glCallbacks[i]));
    }
    if (differential) {
        if (!world::redirect(base + 0x1c35, uintptr_t(&gateCallback)) ||
            !world::redirect(base + 0x13fc, uintptr_t(&meshCallback)) ||
            !world::redirect(base + 0x173f, uintptr_t(&skinCallback)))
            return 101;
        fixtureMode(true);
    }
    FILE *report =
        std::fopen(differential ? "C:/Users/ADMIN/Boxer-lab/ms3d/character-fixtures.jsonl"
                                : "character-offline-fixtures.jsonl",
                   "wb");
    unsigned failures = 0;
    Fixture fixture;
    for (unsigned id = 0; id < caseCount; ++id) {
        fixture.reset(id);
        current = &fixture;
        AbiReport originalAbi{}, candidateAbi{};
        Trace originalTrace;
        std::vector<uint8_t> originalWitness;
        if (report) {
            std::fprintf(report, "{\"case\":%u,\"name\":\"%s\",\"pre\":", id, caseNames[id]);
            fixture.snapshot(report);
        }
        FpPreserver initialEnvironment;
        if (differential) {
            initialEnvironment.restore();
            character_invoke(reinterpret_cast<Entry>(base + 0x69a0), fixture.owner.data(),
                             fixture.model.data(), &originalAbi);
            originalTrace = fixture.trace;
            originalWitness = fixture.witness();
            fixture.reset(id);
        }
        const unsigned routeBefore = differential ? replacementCount() : 0;
        initialEnvironment.restore();
        character_invoke(differential ? reinterpret_cast<Entry>(base + 0x1497) : candidate,
                         fixture.owner.data(), fixture.model.data(), &candidateAbi);
        const bool fpEqual = originalAbi.beforeCW == candidateAbi.beforeCW &&
                             originalAbi.beforeSW == candidateAbi.beforeSW &&
                             originalAbi.beforeMX == candidateAbi.beforeMX &&
                             originalAbi.afterCW == candidateAbi.afterCW &&
                             originalAbi.afterSW == candidateAbi.afterSW &&
                             originalAbi.afterMX == candidateAbi.afterMX;
        const bool equal =
            abiValid(candidateAbi) &&
            (!differential ||
             (abiValid(originalAbi) && fpEqual && originalAbi.result == candidateAbi.result &&
              originalTrace.events == fixture.trace.events &&
              originalWitness == fixture.witness() && replacementCount() == routeBefore + 1));
        if (!equal)
            ++failures;
        if (report) {
            std::fprintf(report,
                         ",\"equal\":%s,\"candidate_result\":%u,\"original_result\":%u,\"actual_"
                         "replacement_calls\":%u,\"candidate_abi\":[",
                         equal ? "true" : "false", candidateAbi.result, originalAbi.result,
                         differential ? replacementCount() - routeBefore : 0);
            for (unsigned i = 0; i < 13; ++i)
                std::fprintf(report, "%s%u", i ? "," : "",
                             reinterpret_cast<uint32_t *>(&candidateAbi)[i]);
            std::fputs("],\"original_abi\":[", report);
            for (unsigned i = 0; i < 13; ++i)
                std::fprintf(report, "%s%u", i ? "," : "",
                             reinterpret_cast<uint32_t *>(&originalAbi)[i]);
            std::fputs("],\"candidate_events\":", report);
            fixture.trace.write(report);
            std::fputs(",\"original_events\":", report);
            originalTrace.write(report);
            std::fputs(",\"post\":", report);
            fixture.snapshot(report);
            std::fputs("}\n", report);
            std::fflush(report);
        }
    }
    if (report)
        std::fclose(report);
    if (differential) {
        fixtureMode(false);
        world::redirect(base + 0x1c35, base + 0x94e0);
        world::redirect(base + 0x13fc, base + 0x9550);
        world::redirect(base + 0x173f, base + 0x9300);
        for (unsigned i = 0; i < 7; ++i)
            writeSlot(image + slots[i], savedSlots[i]);
    }
    current = nullptr;
    currentModule = nullptr;
    return failures;
}
} // namespace
uint32_t fixtures(uintptr_t base) { return run(base, true); }
uint32_t offlineFixtures() { return run(0, false); }
} // namespace character

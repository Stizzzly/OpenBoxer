#include "character_replay.hpp"
#include "character_native.hpp"
#include "character_trace.hpp"
#include "world_runtime.hpp"
#include <windows.h>
#include <array>
#include <algorithm>
namespace character {
namespace {
struct CapturedPose {
    std::array<uint32_t, 10> header{};
    std::array<uint8_t, 80> model{};
    std::array<uint8_t, 288> mesh{};
    std::array<uint32_t, 1> owner{};
    std::vector<uint8_t> positions, normals, uv, triangles;
    Trace trace;
    bool load(const char *path) {
        FILE *f = std::fopen(path, "rb");
        if (!f)
            return false;
        bool ok =
            std::fread(header.data(), 4, 10, f) == 10 && header[0] == 0x36485243 && header[1] == 2;
        const uint32_t frames = header[2], vertices = header[3], count = header[4], a = header[5],
                       b = header[6];
        const uint64_t frameBytes = uint64_t(vertices) * 12,
                       totalFrameBytes = uint64_t(frames) * frameBytes;
        const uint64_t totalBytes =
            totalFrameBytes * 2 + uint64_t(vertices) * 8 + uint64_t(count) * 24;
        ok = ok && frames && vertices && a < frames && b < frames &&
             totalBytes <= native::captureBudget;
        if (ok) {
            positions.resize(size_t(totalFrameBytes));
            normals.resize(size_t(totalFrameBytes));
            uv.resize(size_t(vertices) * 8);
            triangles.resize(size_t(count) * 24);
            ok = std::fread(model.data(), 1, 80, f) == 80 &&
                 std::fread(mesh.data(), 1, 288, f) == 288;
            for (auto *array : {&positions, &normals})
                for (uint32_t frame : {a, b})
                    ok = ok && std::fread(array->data() + frameBytes * frame, 1, size_t(frameBytes),
                                          f) == frameBytes;
            ok = ok && std::fread(uv.data(), 1, uv.size(), f) == uv.size() &&
                 std::fread(triangles.data(), 1, triangles.size(), f) == triangles.size() &&
                 std::fgetc(f) == EOF;
            ok = ok && word(model.data()) == 1 && word(model.data() + 48) == a &&
                 word(model.data() + 52) == b && word(model.data() + 76) == header[7] &&
                 word(mesh.data()) == vertices && word(mesh.data() + 4) == count && mesh[16] == 0;
            for (uint32_t t = 0; ok && t < count; ++t)
                for (unsigned corner = 0; corner < 3; ++corner)
                    ok = word(triangles.data() + 24 * t + 4 * corner) < vertices;
        }
        std::fclose(f);
        if (ok)
            rebase();
        return ok;
    }
    void rebase() {
        put(model.data() + 28, uint32_t(uintptr_t(mesh.data())));
        put(model.data() + 32, uint32_t(uintptr_t(mesh.data() + 288)));
        put(mesh.data() + 272, uint32_t(uintptr_t(positions.data())));
        put(mesh.data() + 276, uint32_t(uintptr_t(normals.data())));
        put(mesh.data() + 280, uint32_t(uintptr_t(uv.data())));
        put(mesh.data() + 284, uint32_t(uintptr_t(triangles.data())));
    }
    std::vector<uint8_t> stateBytes() const {
        FpPreserver preserve;
        std::vector<uint8_t> result;
        result.reserve(80 + 288 + 4 + positions.size() + normals.size() + uv.size() +
                       triangles.size());
        const auto add = [&](const uint8_t *p, size_t n) { result.insert(result.end(), p, p + n); };
        // Runtime pointer words participate in exact equality: a pointer write
        // cannot be hidden by the normalized external witness encoding.
        add(model.data(), 80);
        add(mesh.data(), 288);
        add(reinterpret_cast<const uint8_t *>(owner.data()), 4);
        add(positions.data(), positions.size());
        add(normals.data(), normals.size());
        add(uv.data(), uv.size());
        add(triangles.data(), triangles.size());
        return result;
    }
    bool save(const char *path) const {
        FILE *f = std::fopen(path, "wb");
        if (!f)
            return false;
        uint8_t modelBytes[80], meshBytes[288];
        std::memcpy(modelBytes, model.data(), 80);
        std::memcpy(meshBytes, mesh.data(), 288);
        for (unsigned offset : {12u, 16u, 20u, 28u, 32u, 36u})
            put(modelBytes + offset, 0);
        for (unsigned offset : {272u, 276u, 280u, 284u})
            put(meshBytes + offset, 0);
        bool ok = std::fwrite(header.data(), 4, 10, f) == 10 &&
                  std::fwrite(modelBytes, 1, 80, f) == 80 &&
                  std::fwrite(meshBytes, 1, 288, f) == 288;
        const size_t frameBytes = size_t(header[3]) * 12;
        for (const auto *array : {&positions, &normals})
            for (uint32_t frame : {header[5], header[6]})
                ok = ok && std::fwrite(array->data() + frameBytes * frame, 1, frameBytes, f) ==
                               frameBytes;
        ok = ok && std::fwrite(uv.data(), 1, uv.size(), f) == uv.size() &&
             std::fwrite(triangles.data(), 1, triangles.size(), f) == triangles.size();
        return std::fclose(f) == 0 && ok;
    }
};
thread_local CapturedPose *pose = nullptr;
uint8_t *image = nullptr;
uint32_t __attribute__((thiscall)) gate(void *collection) {
    const uint32_t result = collection == pose->model.data() + 24 ? 1 : 0;
    pose->trace.append("mesh_gate", nullptr, 0, "model.mesh_collection", result);
    return result;
}
void *__attribute__((thiscall)) mesh(void *, int32_t index) {
    uint32_t arg = uint32_t(index);
    pose->trace.append("mesh_lookup", &arg, 1, "model.mesh_collection", 0,
                       index == 0 ? "mesh0" : "UNKNOWN");
    return pose->mesh.data();
}
void *__attribute__((thiscall)) skin(void *, int32_t index) {
    uint32_t arg = uint32_t(index);
    pose->trace.append("UNEXPECTED_skin_lookup", &arg, 1, "model.skin_collection");
    return nullptr;
}
uint32_t __stdcall enable(uint32_t a) {
    pose->trace.append("glEnable", &a, 1);
    return 0xe1;
}
uint32_t __stdcall bind(uint32_t a, uint32_t b) {
    uint32_t args[] = {a, b};
    pose->trace.append("glBindTexture", args, 2);
    return 0xb1;
}
uint32_t __stdcall begin(uint32_t a) {
    pose->trace.append("glBegin", &a, 1);
    return 0xbe;
}
uint32_t __stdcall uv(uint32_t a, uint32_t b) {
    uint32_t args[] = {a, b};
    pose->trace.append("glTexCoord2f", args, 2);
    return 0x75;
}
uint32_t __stdcall normal(uint32_t x, uint32_t y, uint32_t z) {
    uint32_t args[] = {x, y, z};
    pose->trace.append("glNormal3f", args, 3);
    return 0x6e;
}
uint32_t __stdcall vertex(uint32_t x, uint32_t y, uint32_t z) {
    uint32_t args[] = {x, y, z};
    pose->trace.append("glVertex3f", args, 3);
    return 0x76;
}
uint32_t __stdcall end() {
    pose->trace.append("glEnd", nullptr, 0);
    return 0xed;
}
const uintptr_t recorders[] = {uintptr_t(&enable), uintptr_t(&bind),   uintptr_t(&begin),
                               uintptr_t(&uv),     uintptr_t(&normal), uintptr_t(&vertex),
                               uintptr_t(&end)};
uint32_t __attribute__((thiscall)) candidate(void *owner, void *model) {
    Dependencies d{gate, mesh, skin, image};
    BinarySource source(owner, model, d);
    BinaryOutput output(d);
    return render(source, output);
}
bool abiValid(const AbiReport &a) {
    return a.beforeStack == a.afterStack && a.ebx == 0x11223344 && a.esi == 0x22334455 &&
           a.edi == 0x33445566 && a.ebp == 0x44556677 && a.beforeCW == a.afterCW &&
           (a.beforeMX & 0xffc0) == (a.afterMX & 0xffc0);
}
bool fpEqual(const AbiReport &a, const AbiReport &b) {
    return a.beforeCW == b.beforeCW && a.beforeSW == b.beforeSW && a.beforeMX == b.beforeMX &&
           a.afterCW == b.afterCW && a.afterSW == b.afterSW && a.afterMX == b.afterMX;
}
bool writeSlot(void *p, uint32_t v) {
    DWORD old, unused;
    if (!VirtualProtect(p, 4, PAGE_READWRITE, &old))
        return false;
    put(p, v);
    return VirtualProtect(p, 4, old, &unused) != 0;
}
bool compare(const char *capture, FILE *report, uintptr_t base) {
    CapturedPose captureData;
    if (!captureData.load(capture))
        return false;
    pose = &captureData;
    FpPreserver initial;
    const std::vector<uint8_t> beforeBytes = captureData.stateBytes();
    AbiReport originalAbi{}, candidateAbi{};
    Trace originalTrace;
    const std::string witnessPrefix = std::string(capture) + "-replay";
    const bool preSaved = captureData.save((witnessPrefix + "-pre.bin").c_str());
    if (base) {
        initial.restore();
        character_invoke(reinterpret_cast<Entry>(base + 0x69a0), captureData.owner.data(),
                         captureData.model.data(), &originalAbi);
        originalTrace = std::move(captureData.trace);
    }
    const bool exactOriginal = beforeBytes == captureData.stateBytes();
    const bool originalSaved = captureData.save((witnessPrefix + "-original-post.bin").c_str());
    const unsigned routeBefore = base ? replacementCount() : 0;
    initial.restore();
    character_invoke(base ? reinterpret_cast<Entry>(base + 0x1497) : candidate,
                     captureData.owner.data(), captureData.model.data(), &candidateAbi);
    const bool exactCandidate = beforeBytes == captureData.stateBytes();
    const bool candidateSaved = captureData.save((witnessPrefix + "-candidate-post.bin").c_str());
    const unsigned routeDelta = base ? replacementCount() - routeBefore : 0;
    const bool equal =
        abiValid(candidateAbi) && exactOriginal && exactCandidate && preSaved && originalSaved &&
        candidateSaved &&
        (!base || (abiValid(originalAbi) && fpEqual(originalAbi, candidateAbi) &&
                   originalAbi.result == candidateAbi.result &&
                   originalTrace.events == captureData.trace.events && routeDelta == 1));
    if (report) {
        std::fprintf(
            report,
            "{\"snapshot_version\":2,\"original_model_identity\":%u,\"original_owner_identity\":%u,"
            "\"frame_bounds\":%u,\"vertices_per_frame\":%u,\"triangles\":%u,\"frame_a\":%u,\"frame_"
            "b\":%u,\"factor_bits\":%u,\"equal\":%s,\"candidate_result\":%u,\"original_result\":%u,"
            "\"actual_replacement_calls\":%u,\"state_exact_unchanged\":%s,\"candidate_abi\":[",
            captureData.header[9], captureData.header[8], captureData.header[2],
            captureData.header[3], captureData.header[4], captureData.header[5],
            captureData.header[6], captureData.header[7], equal ? "true" : "false",
            candidateAbi.result, originalAbi.result, routeDelta,
            exactOriginal && exactCandidate ? "true" : "false");
        for (unsigned i = 0; i < 13; ++i)
            std::fprintf(report, "%s%u", i ? "," : "",
                         reinterpret_cast<const uint32_t *>(&candidateAbi)[i]);
        std::fputs("],\"original_abi\":[", report);
        for (unsigned i = 0; i < 13; ++i)
            std::fprintf(report, "%s%u", i ? "," : "",
                         reinterpret_cast<const uint32_t *>(&originalAbi)[i]);
        std::fputs("],\"candidate_events\":", report);
        captureData.trace.write(report);
        std::fputs(",\"original_events\":", report);
        originalTrace.write(report);
        std::fputs("}\n", report);
        std::fflush(report);
    }
    pose = nullptr;
    return equal;
}
} // namespace
uint32_t replays(uintptr_t base) {
    FpPreserver preserve;
    uint16_t cw = 0x027f;
    __asm__ volatile("fldcw %0" ::"m"(cw));
    image = reinterpret_cast<uint8_t *>(base);
    uint32_t saved[7];
    for (unsigned i = 0; i < 7; ++i) {
        saved[i] = word(image + slots[i]);
        if (!writeSlot(image + slots[i], uint32_t(recorders[i])))
            return 100;
    }
    if (!world::redirect(base + 0x1c35, uintptr_t(&gate)) ||
        !world::redirect(base + 0x13fc, uintptr_t(&mesh)) ||
        !world::redirect(base + 0x173f, uintptr_t(&skin)))
        return 101;
    fixtureMode(true);
    FILE *report = std::fopen("C:/Users/ADMIN/Boxer-lab/ms3d/character-replays.jsonl", "wb");
    unsigned failures = 0, count = 0;
    for (const char *route : {"original", "replacement"})
        for (unsigned call : {1u, 2u, 3u, 120u, 240u}) {
            char path[256];
            std::snprintf(path, sizeof(path),
                          "C:/Users/ADMIN/Boxer-lab/ms3d/character-native-%s-%04u-pre.bin", route,
                          call);
            if (GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES) {
                ++count;
                if (!compare(path, report, base))
                    ++failures;
            }
        }
    if (report)
        std::fclose(report);
    fixtureMode(false);
    world::redirect(base + 0x1c35, base + 0x94e0);
    world::redirect(base + 0x13fc, base + 0x9550);
    world::redirect(base + 0x173f, base + 0x9300);
    for (unsigned i = 0; i < 7; ++i)
        writeSlot(image + slots[i], saved[i]);
    image = nullptr;
    return count ? failures : 102;
}
uint32_t offlineReplay(const char *capture, const char *reportPath) {
    FpPreserver preserve;
    uint16_t cw = 0x027f;
    __asm__ volatile("fldcw %0" ::"m"(cw));
    std::vector<uint8_t> storage(0x19f000);
    image = storage.data();
    for (unsigned i = 0; i < 7; ++i)
        put(image + slots[i], uint32_t(recorders[i]));
    FILE *report = std::fopen(reportPath, "wb");
    if (!report)
        return 103;
    const bool equal = compare(capture, report, 0);
    std::fclose(report);
    image = nullptr;
    return equal ? 0 : 1;
}
} // namespace character

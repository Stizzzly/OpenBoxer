#include "character_trace.hpp"
#include "character_native.hpp"
#include "draw_trace.hpp"
#include "world_runtime.hpp"
#include "runtime_options.hpp"
#include <windows.h>
namespace character {
namespace {
uintptr_t module = 0;
std::string mode;
unsigned naturalCalls = 0, unsupportedCalls = 0;
bool isolated = false;
unsigned replacementCalls = 0;
thread_local Trace *active = nullptr;
uint32_t originalNormal = 0, originalVertex = 0;
bool readable(const void *ptr, size_t size) {
    uintptr_t p = reinterpret_cast<uintptr_t>(ptr);
    if (!p || uint64_t(p) + size > 0xffffffffULL)
        return false;
    while (size) {
        MEMORY_BASIC_INFORMATION m{};
        if (!VirtualQuery(reinterpret_cast<void *>(p), &m, sizeof(m)) || m.State != MEM_COMMIT ||
            (m.Protect & (PAGE_NOACCESS | PAGE_GUARD)))
            return false;
        const uintptr_t end = reinterpret_cast<uintptr_t>(m.BaseAddress) + m.RegionSize;
        const size_t step = (std::min)(size, size_t(end - p));
        if (!step)
            return false;
        p += step;
        size -= step;
    }
    return true;
}
bool executable(uint32_t address) {
    MEMORY_BASIC_INFORMATION m{};
    return address && VirtualQuery(reinterpret_cast<void *>(uintptr_t(address)), &m, sizeof(m)) &&
           m.State == MEM_COMMIT && !(m.Protect & (PAGE_NOACCESS | PAGE_GUARD)) &&
           (m.Protect &
            (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY));
}
void sharedObservation(draw::Api api, const uint32_t *args, unsigned count) {
    if (!active)
        return;
    const char *name = nullptr;
    switch (api) {
    case draw::Api::Enable:
        name = "glEnable";
        break;
    case draw::Api::BindTexture:
        name = "glBindTexture";
        break;
    case draw::Api::Begin:
        name = "glBegin";
        break;
    case draw::Api::TexCoord:
        name = "glTexCoord2f";
        break;
    case draw::Api::End:
        name = "glEnd";
        break;
    default:
        return;
    }
    active->append(name, args, count);
}
uint32_t __stdcall normal(uint32_t x, uint32_t y, uint32_t z) {
    if (active) {
        uint32_t args[] = {x, y, z};
        active->append("glNormal3f", args, 3);
    }
    return reinterpret_cast<Three>(uintptr_t(originalNormal))(x, y, z);
}
uint32_t __stdcall vertex(uint32_t x, uint32_t y, uint32_t z) {
    if (active) {
        uint32_t args[] = {x, y, z};
        active->append("glVertex3f", args, 3);
    }
    return reinterpret_cast<Three>(uintptr_t(originalVertex))(x, y, z);
}
bool writeSlot(void *ptr, uint32_t value) {
    DWORD old, unused;
    if (!VirtualProtect(ptr, 4, PAGE_READWRITE, &old))
        return false;
    put(ptr, value);
    return VirtualProtect(ptr, 4, old, &unused) != 0;
}
uint32_t __attribute__((thiscall)) entry(void *owner, void *model) {
    runtime_options::Timing timing(runtime_options::Unit::Character);
    if (isolated) {
        Dependencies d{
            reinterpret_cast<Gate>(module + 0x1c35), reinterpret_cast<Lookup>(module + 0x13fc),
            reinterpret_cast<Lookup>(module + 0x173f), reinterpret_cast<uint8_t *>(module)};
        BinarySource source(owner, model, d);
        BinaryOutput output(d);
        ++replacementCalls;
        return render(source, output);
    }
    const uintptr_t caller = reinterpret_cast<uintptr_t>(__builtin_return_address(0));
    const bool natural = caller == module + 0x5fef;
    const unsigned call = natural ? ++naturalCalls : ++unsupportedCalls;
    native::Registration registration{};
    const char *reason = "unmatched-caller";
    timing.checkpoint("guard");
    const bool allowed = natural && native::supported(owner, model, registration, reason);
    const bool replacement = allowed && mode == "replace";
    const bool capture = runtime_options::capture(runtime_options::Unit::Character) && allowed && (call <= 3 || call == 120 || call == 240);
    const char *route = replacement ? "replacement" : allowed ? "original" : "original-fallback";
    Trace trace;
    uint32_t preCount = 0, preA = 0, preB = 0, preFactor = 0;
    uint16_t cw = 0, sw = 0;
    uint32_t mx = 0;
    bool snapshotComplete = false;
    char path[256];
    if (capture) {
        FpPreserver preserve;
        ModelView source{model};
        preCount = uint32_t(source.meshCount());
        preA = uint32_t(source.frame(false));
        preB = uint32_t(source.frame(true));
        preFactor = source.factor();
        std::snprintf(path, sizeof(path),
                      "C:/Users/ADMIN/Boxer-lab/ms3d/character-native-%s-%04u-pre.bin", route,
                      call);
        snapshotComplete = native::snapshot(path, registration, owner);
        active = &trace;
    }
    __asm__ volatile("fnstcw %0" : "=m"(cw));
    __asm__ volatile("fnstsw %0" : "=m"(sw));
    __asm__ volatile("stmxcsr %0" : "=m"(mx));
    uint32_t result;
    timing.checkpoint("render");
    if (replacement) {
        Dependencies d{
            reinterpret_cast<Gate>(module + 0x1c35), reinterpret_cast<Lookup>(module + 0x13fc),
            reinterpret_cast<Lookup>(module + 0x173f), reinterpret_cast<uint8_t *>(module)};
        BinarySource source(owner, model, d);
        BinaryOutput output(d);
        ++replacementCalls;
        result = render(source, output);
    } else
        result = reinterpret_cast<Entry>(module + 0x69a0)(owner, model);
    active = nullptr;
    uint16_t afterCW = 0, afterSW = 0;
    uint32_t afterMX = 0;
    __asm__ volatile("fnstcw %0" : "=m"(afterCW));
    __asm__ volatile("fnstsw %0" : "=m"(afterSW));
    __asm__ volatile("stmxcsr %0" : "=m"(afterMX));
    timing.checkpoint("render-complete");
    if (capture || (runtime_options::diagnostics(runtime_options::Unit::Character) && natural && !allowed && (call <= 3 || call == 120 || call == 240))) {
        FpPreserver preserve;
        FILE *f = std::fopen("C:/Users/ADMIN/Boxer-lab/ms3d/character-native.jsonl", "ab");
        if (f) {
            std::fprintf(f,
                         "{\"unit\":\"RENDER-0006\",\"route\":\"%s\",\"reason\":\"%s\",\"call\":%u,"
                         "\"caller_rva\":%u,\"owner_identity\":%u,\"model_identity\":%u,\"mesh_"
                         "identity\":%u,\"mesh_count\":%u,\"frame_a\":%u,\"frame_b\":%u,\"factor_"
                         "bits\":%u,\"cw\":%u,\"sw\":%u,\"mxcsr\":%u,\"after_cw\":%u,\"after_sw\":%"
                         "u,\"after_mxcsr\":%u,\"result\":%u,\"actual_replacements\":%u,\"typed_"
                         "snapshot_complete\":%s,\"asset_sha256\":\"%s\",\"events\":",
                         route, reason, call, uint32_t(caller - module), uint32_t(uintptr_t(owner)),
                         uint32_t(uintptr_t(model)), uint32_t(uintptr_t(registration.mesh)),
                         preCount, preA, preB, preFactor, cw, sw, mx, afterCW, afterSW, afterMX,
                         result, replacementCalls, snapshotComplete ? "true" : "false",
                         registration.asset ? registration.asset->sha256 : "UNKNOWN");
            trace.write(f);
            std::fputs("}\n", f);
            std::fclose(f);
        }
        if (capture) {
            std::snprintf(path, sizeof(path),
                          "C:/Users/ADMIN/Boxer-lab/ms3d/character-native-%s-%04u-post.bin", route,
                          call);
            native::snapshot(path, registration, owner);
        }
    }
    return result;
}
} // namespace
bool install(uintptr_t base, const char *selected) {
    module = base;
    mode = selected;
    auto *image = reinterpret_cast<uint8_t *>(base);
    originalNormal = word(image + slots[unsigned(Api::Normal)]);
    originalVertex = word(image + slots[unsigned(Api::Vertex)]);
    if (!executable(originalNormal) || !executable(originalVertex))
        return false;
    // Existing map wrappers own all shared slots. Character observes those
    // wrappers through a tag; only the two scalar XYZ slots have new wrappers.
    if(runtime_options::capture(runtime_options::Unit::Character)) {
        draw::setObserver(sharedObservation);
        if (!writeSlot(image + slots[unsigned(Api::Normal)], uint32_t(uintptr_t(&normal))) ||
            !writeSlot(image + slots[unsigned(Api::Vertex)], uint32_t(uintptr_t(&vertex))))
            return false;
    }
    return world::redirect(base + 0x1497, reinterpret_cast<uintptr_t>(&entry)) &&
           native::install(base);
}
void fixtureMode(bool value) { isolated = value; }
unsigned replacementCount() { return replacementCalls; }
} // namespace character

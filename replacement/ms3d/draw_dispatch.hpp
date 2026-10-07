#pragma once
#include "draw_model.hpp"
namespace draw {
// Stack words preserve float bits; approved x86 stdcall ABI uses these same words.
using One = uint32_t(__stdcall *)(uint32_t);
using Two = uint32_t(__stdcall *)(uint32_t, uint32_t);
using Three = uint32_t(__stdcall *)(uint32_t, uint32_t, uint32_t);
using Pointer = uint32_t(__stdcall *)(const uint32_t *);
using MaterialVector = uint32_t(__stdcall *)(uint32_t, uint32_t, const uint32_t *);
using Vector = uint32_t(__stdcall *)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
using Zero = uint32_t(__stdcall *)();
enum class Api : unsigned {
    IsEnabled,
    Materialfv,
    Materialf,
    BindTexture,
    Enable,
    Disable,
    Begin,
    Normal,
    TexCoord,
    Vertex,
    End,
    ShaderVector,
    ShaderScalar,
    Count
};
constexpr uint32_t slots[] = {0x18caec, 0x18cae8, 0x18cae4, 0x18ca58, 0x18ca68, 0x18cab8, 0x18ca8c,
                              0x18cadc, 0x18ca88, 0x18cad8, 0x18ca7c, 0x1855cc, 0x1855d8};
constexpr uint32_t vectorHandles[] = {0x1850e0, 0x17564c, 0x17d900, 0x1842b4},
                   scalarHandle = 0x176a14, mapGlobal = 0x185560;
struct Dispatch {
    uint8_t *module;
    template <class T> T get(Api api) const {
        return reinterpret_cast<T>(readWord(module + slots[static_cast<unsigned>(api)]));
    }
    uint32_t handle(unsigned group) const {
        return readWord(module + vectorHandles[group]);
    }
    uint32_t shininessHandle() const {
        return readWord(module + scalarHandle);
    }
};
}

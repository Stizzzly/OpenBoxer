#pragma once
#include <cstdint>
namespace character {
struct ApprovedAsset {
    const char *path, *sha256;
    uint32_t frames, vertices, triangles;
};
inline constexpr ApprovedAsset approvedAssets[] = {
    {"base\\fighters\\1_lower.bhm",
     "CCE184703AE8145DAA8ADAAD06D421E1D6440E682BFB7A08DBFA952B9D09FDEC", 749, 1999, 2832},
    {"base\\fighters\\2_lower.bhm",
     "2D53125B347177FC466DF987BDC003480A1FE4AFB6CB24794DC60FED0320CEF1", 749, 1944, 2794},
    {"base\\fighters\\3_lower.bhm",
     "E525DD0403BBF0A88D93C3E2F06DC2A62C6BE5EF2F2846EF9E661CD40BAAD36D", 749, 2036, 2714},
    {"base\\fighters\\4_lower.bhm",
     "153CB5F56EA9BE28EEF991724E0A5FD6FEDEC4CA20B1EC37A6169BEC6C41F667", 749, 2003, 2827},
    {"base\\fighters\\5_lower.bhm",
     "0BCE8E551F0FE21511DA2F41EC5C9A95387CE6FDCEFB87519A15BFFC5316BC49", 749, 2462, 2878},
    {"base\\fighters\\6_lower.bhm",
     "28FF235E66D6366F070BE276E3ED0E4AB39CE0AD37340FC404B74249C5BB4576", 749, 2071, 2962},
    {"base\\fighters\\7_lower.bhm",
     "4B42860245BC53CD9A1ABA84404F5DC15E881BAF834EA3D1D873ACC2C5BDFE4E", 749, 2098, 2836},
    {"base\\fighters\\8_lower.bhm",
     "04C4935B574491F91F5B24DDFE115287790BADA359BD2D3DE3E8CFD977851B38", 749, 2178, 2786},
};
} // namespace character

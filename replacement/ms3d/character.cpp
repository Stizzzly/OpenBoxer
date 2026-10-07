#include "character.hpp"
namespace character {
FloatBits interpolate(FloatBits a, FloatBits b, FloatBits factor) {
    FloatBits result;
    // Semantic x87 adapter: retain active precision/rounding and store only
    // the final argument to float32. No compiler reassociation or FMA.
    __asm__ volatile("flds %2\n\tfsubs %1\n\tfmuls %3\n\tfadds %1\n\tfstps %0"
                     : "=m"(result)
                     : "m"(a), "m"(b), "m"(factor)
                     : "st");
    return result;
}
static Triple blend(Triple a, Triple b, const Source &source) {
    Triple result;
    result.z = interpolate(a.z, b.z, source.factor());
    result.y = interpolate(a.y, b.y, source.factor());
    result.x = interpolate(a.x, b.x, source.factor());
    return result;
}
uint32_t render(Source &source, Output &output) {
    const uint32_t gate = source.meshGate();
    if (!gate)
        return 0;
    int32_t meshIndex = 0;
    if (meshIndex >= source.meshCount())
        return gate;
    do {
        const MeshHandle mesh = source.mesh(meshIndex);
        const uint32_t frameA = source.frameOffset(mesh, false);
        const uint32_t frameB = source.frameOffset(mesh, true);
        if (source.textured(mesh)) {
            output.enableTexture();
            output.bindTexture(source.texture(mesh));
        }
        output.beginTriangles();
        for (int32_t triangle = 0; triangle < source.triangleCount(mesh); ++triangle) {
            for (int corner = 2; corner >= 0; --corner) {
                const int32_t index = source.cornerIndex(mesh, triangle, unsigned(corner));
                FloatBits s, t;
                if (source.uv(mesh, index, s, t))
                    output.texCoord(s, t);
                const CornerPose pose = source.pose(mesh, frameA, frameB, index);
                output.normal(blend(pose.normalA, pose.normalB, source));
                output.vertex(blend(pose.positionA, pose.positionB, source));
            }
        }
        output.end();
        ++meshIndex;
    } while (meshIndex < source.meshCount());
    return uint32_t(meshIndex);
}
} // namespace character

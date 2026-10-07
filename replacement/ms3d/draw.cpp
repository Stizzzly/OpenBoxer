#include "draw.hpp"
namespace draw {
namespace {
constexpr uint32_t texture2d = 0x0de1, front = 0x0404;
void configureMaterial(ModelView model, int32_t selected, const Dispatch &gl) {
    const uint32_t names[] = {0x1200, 0x1201, 0x1202, 0x1600};
    for (unsigned group = 0; group < 4; ++group) {
        const auto *values =
            reinterpret_cast<const uint32_t *>(&model.material(selected)) + 4 * group;
        gl.get<MaterialVector>(Api::Materialfv)(front, names[group], values);
    }
    const uint32_t shininess = model.material(selected).shininess;
    gl.get<Three>(Api::Materialf)(front, 0x1601, shininess);
    for (unsigned group = 0; group < 4; ++group) {
        // The specified read order is blue, green, red, then live handle and slot.
        const uint32_t blue =
            reinterpret_cast<const uint32_t *>(&model.material(selected))[4 * group + 2];
        const uint32_t green =
            reinterpret_cast<const uint32_t *>(&model.material(selected))[4 * group + 1];
        const uint32_t red =
            reinterpret_cast<const uint32_t *>(&model.material(selected))[4 * group];
        const uint32_t handle = gl.handle(group);
        gl.get<Vector>(Api::ShaderVector)(handle, red, green, blue, 0x3f800000);
    }
    const uint32_t scalar = model.material(selected).shininess;
    const uint32_t handle = gl.shininessHandle();
    gl.get<Two>(Api::ShaderScalar)(handle, scalar);
}
void configureTexture(ModelView model, int32_t selected, const Dispatch &gl) {
    if (selected >= 0 && model.material(selected).textureId != 0) {
        const uint32_t texture = model.material(selected).textureId;
        gl.get<Two>(Api::BindTexture)(texture2d, texture);
        gl.get<One>(Api::Enable)(texture2d);
    } else
        gl.get<One>(Api::Disable)(texture2d);
}
void emitTriangle(ModelView model, Triangle *triangle, const Dispatch &gl) {
    for (unsigned corner = 0; corner < 3; ++corner) {
        const int32_t vertexIndex = triangle->vertices[corner];
        gl.get<Pointer>(Api::Normal)(triangle->normals[corner]);
        const uint32_t t = triangle->t[corner], s = triangle->s[corner];
        gl.get<Two>(Api::TexCoord)(s, t);
        gl.get<Pointer>(Api::Vertex)(model.vertices()[vertexIndex].position);
    }
}
void emitMemberships(ModelView model, int32_t meshIndex, const Dispatch &gl) {
    gl.get<One>(Api::Begin)(4);
    for (int32_t member = 0; member < model.mesh(meshIndex).membershipCount; ++member) {
        const int32_t triangleIndex = model.mesh(meshIndex).memberships[member];
        Triangle *triangle = &model.triangles()[triangleIndex];
        emitTriangle(model, triangle, gl);
    }
    gl.get<Zero>(Api::End)();
}
}
uint32_t render(ModelView model, const Dispatch &gl) {
    const uint8_t entryTextureEnabled =
        static_cast<uint8_t>(gl.get<One>(Api::IsEnabled)(texture2d));
    for (int32_t meshIndex = 0; meshIndex < model.meshCount(); ++meshIndex) {
        const int32_t selected = model.mesh(meshIndex).material;
        if (selected >= 0)
            configureMaterial(model, selected, gl);
        configureTexture(model, selected, gl);
        if (draw_above_threshold(model.material(selected).transparency))
            emitMemberships(model, meshIndex, gl);
    }
    return gl.get<One>(entryTextureEnabled ? Api::Enable : Api::Disable)(texture2d);
}
}

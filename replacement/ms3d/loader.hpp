#pragma once
#include <cstdint>
#include <cstddef>
#include <vector>
namespace ms3d {
constexpr std::size_t objectSize=0x8d9c4;
constexpr std::size_t fieldsOffset=0x8d9a4;
struct Fields { int32_t groups; void* groupData; int32_t materials; void* materialData; int32_t triangles; void* triangleData; int32_t vertices; void* vertexData; };
static_assert(sizeof(Fields)==32 && sizeof(void*)==4);
static_assert(offsetof(Fields,groupData)==4 && offsetof(Fields,materialData)==12 && offsetof(Fields,triangleData)==20 && offsetof(Fields,vertexData)==28);
using Allocate=void* (__cdecl*)(uint32_t);
using Release=void (__cdecl*)(void*);
struct Allocator { Allocate allocate; Release release; };
struct Group { int32_t material; std::vector<uint32_t> indices; };
struct Parsed { uint32_t vertices=0,triangles=0,materials=0; std::size_t vertexOffset=0,triangleOffset=0,materialOffset=0; std::vector<Group> groups; };
bool parse(const std::vector<uint8_t>& bytes, Parsed& result);
bool install(void* object,const std::vector<uint8_t>& bytes,const Parsed& parsed,Allocator allocator);
void cleanup(void* object,Allocator allocator);
Fields& fields(void* object);
}

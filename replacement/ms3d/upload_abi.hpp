#pragma once
#include "upload.hpp"
#include "world_abi.hpp"
extern "C" uint32_t __cdecl upload_abi_probe(upload::Entry,const char*,WorldAbiReport*);

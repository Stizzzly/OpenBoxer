#pragma once
#include "character_layout.hpp"
#include <cstdio>
namespace character {
struct FpPreserver {
    alignas(16) uint8_t state[512];
    FpPreserver() { __asm__ volatile("fxsave %0" : "=m"(state)); }
    ~FpPreserver() { __asm__ volatile("fxrstor %0" ::"m"(state)); }
    void restore() const { __asm__ volatile("fxrstor %0" ::"m"(state)); }
};
struct Event {
    std::string name, role, resultRole;
    std::vector<uint32_t> args;
    uint32_t result = 0;
    bool operator==(const Event &b) const {
        return name == b.name && role == b.role && resultRole == b.resultRole && args == b.args &&
               result == b.result;
    }
};
struct Trace {
    std::vector<Event> events;
    void append(const char *name, const uint32_t *args, unsigned count,
                const std::string &role = "", uint32_t result = 0,
                const std::string &resultRole = "") {
        FpPreserver preserve;
        Event event{name, role, resultRole, {}, result};
        if (count)
            event.args.assign(args, args + count);
        events.push_back(std::move(event));
    }
    void write(FILE *f) const {
        std::fputs("[", f);
        for (size_t i = 0; i < events.size(); ++i) {
            const auto &e = events[i];
            std::fprintf(f,
                         "%s{\"name\":\"%s\",\"role\":\"%s\",\"result_role\":\"%s\",\"result\":%u,"
                         "\"args\":[",
                         i ? "," : "", e.name.c_str(), e.role.c_str(), e.resultRole.c_str(),
                         e.result);
            for (size_t j = 0; j < e.args.size(); ++j)
                std::fprintf(f, "%s%u", j ? "," : "", e.args[j]);
            std::fputs("]}", f);
        }
        std::fputs("]", f);
    }
};
} // namespace character

#pragma once
#include <string>
namespace character {
// Approved lexical equivalence only: separators/case and leading .\.
// Traversal is rejected rather than resolved; no basename/suffix matching.
inline bool normalizeAssetPath(const std::string &input, std::string &output) {
    output.clear();
    if (input.empty() || input.size() > 511)
        return false;
    for (unsigned char c : input) {
        if (c == 0 || c >= 128 || c < 32)
            return false;
        if (c == '/')
            c = '\\';
        if (c >= 'A' && c <= 'Z')
            c = static_cast<unsigned char>(c - 'A' + 'a');
        if (c == '\\' && !output.empty() && output.back() == '\\')
            continue;
        output += char(c);
    }
    while (output.size() >= 2 && output[0] == '.' && output[1] == '\\')
        output.erase(0, 2);
    if (output.empty())
        return false;
    size_t start = 0;
    while (start <= output.size()) {
        const size_t end = output.find('\\', start);
        const std::string component =
            output.substr(start, end == std::string::npos ? std::string::npos : end - start);
        if (component == ".." || component == ".")
            return false;
        if (end == std::string::npos)
            break;
        start = end + 1;
    }
    return true;
}
} // namespace character

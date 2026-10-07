#include "frames.hpp"
#include "frames_continuation.hpp"
int main() {
    // Admission is single use, model-specific, and unavailable outside scope.
    int model,other;
    if(animation::continuation::consume(&model))return 1;
    { animation::continuation::Scope scope(&model); if(animation::continuation::consume(&other))return 2; if(!animation::continuation::consume(&model))return 3; if(animation::continuation::consume(&model))return 4; }
    if(animation::continuation::consume(&model))return 5;
    return int(frames::offlineFixtures("frames-offline-fixtures.jsonl"));
}

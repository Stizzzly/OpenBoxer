#pragma once
#include "clip_trace.hpp"
namespace action {
struct Pending { void *model=nullptr; unsigned role=0,sequence=0; bool replacement=false; uint32_t selector=0,frame=0,anchor=0,begin=0,end=0; };
inline thread_local Pending pending;
inline thread_local bool composing=false;
inline void selected(void *model,unsigned role,unsigned sequence,bool replacement) {
 auto *m=static_cast<uint8_t*>(model);
 pending={model,role,sequence,replacement,clip::word(m+44),clip::word(m+48),clip::word(m+80),clip::word(m+88),clip::word(m+92)};
}
inline bool waiting(void *model) {
 if(composing || pending.model!=model)return false;
 auto *m=static_cast<uint8_t*>(model);
 return pending.selector==clip::word(m+44) && pending.frame==clip::word(m+48) && pending.anchor==clip::word(m+80) && pending.begin==clip::word(m+88) && pending.end==clip::word(m+92);
}
struct CompositionScope { bool active; explicit CompositionScope(bool enabled):active(enabled) { if(active)composing=true; } ~CompositionScope() { if(active)composing=false; } };
inline void completed(void *model,const void *before,const void *prepared,const void *after,uint32_t eax,unsigned framesBefore,unsigned framesAfter,unsigned animationBefore,unsigned animationAfter,unsigned continuations,unsigned framesSequence,bool framesReplacement) {
 if(pending.model!=model)return;
 character::FpPreserver fp;
 const Pending witness=pending; pending={};
 std::array<uint8_t,112> a{},b{},c{};
 std::memcpy(a.data(),before,112); std::memcpy(b.data(),prepared,112); std::memcpy(c.data(),after,112);
 clip::normalize(a.data()); clip::normalize(b.data()); clip::normalize(c.data());
 FILE *f=std::fopen("C:/Users/ADMIN/Boxer-lab/ms3d/action-composed.jsonl","ab");
 if(f) {
  std::fprintf(f,"{\"unit\":\"ANIM-0004\",\"role_state\":%u,\"selector_call\":%u,\"selector_replacement\":%s,\"model_role\":\"owner+644\",\"frames_replacements_delta\":%u,\"animation_replacements_delta\":%u,\"trusted_continuations_delta\":%u,\"eax\":%u,\"before_model_words\":",witness.role,witness.sequence,witness.replacement?"true":"false",framesAfter-framesBefore,animationAfter-animationBefore,continuations,eax);
  clip::words(f,a.data()); std::fputs(",\"prepared_model_words\":",f); clip::words(f,b.data()); std::fputs(",\"after_model_words\":",f); clip::words(f,c.data());
  std::fprintf(f,",\"frames_call\":%u,\"frames_route\":\"%s\",\"pose_file\":\"action-pose-%s-%04u.bin\",\"selected_index\":%u,\"selected_anchor_bits\":%u}\n",framesSequence,framesReplacement?"replacement":"original",framesReplacement?"replacement":"original",framesSequence,witness.selector,witness.anchor);
  std::fclose(f);
 }
}
}

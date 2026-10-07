#include "selection.hpp"
#include <cstring>
namespace selection {
static thread_local Observer observer=nullptr;
static thread_local void* observerContext=nullptr;
void setObserver(Observer value,void* context){observer=value;observerContext=context;}
uint32_t bits(const void* p) { return *static_cast<const volatile uint32_t*>(p); }
void put(void* p,uint32_t v) { *static_cast<volatile uint32_t*>(p)=v; }
namespace {
int32_t integer(const void* p) { return static_cast<int32_t>(bits(p)); }
Vector scratch() { return {0xcccccccc,0xcccccccc,0xcccccccc,0xcccccccc}; }
uint32_t difference(uint32_t a,uint32_t b,bool y,bool reverse=false) {
 uint32_t out; const uint32_t k=0x40133333;
 if(reverse) { const auto t=a; a=b; b=t; }
 if(y) __asm__ volatile("flds %1; fsubs %2; fadds %3; fstps %0":"=m"(out):"m"(a),"m"(b),"m"(k):"st");
 else __asm__ volatile("flds %1; fsubs %2; fstps %0":"=m"(out):"m"(a),"m"(b):"st");
 return out;
}
uint32_t contribution(uint32_t origin,uint32_t position,bool y,uint32_t previousDifference,uint32_t sum,bool add) {
 uint32_t out; const uint32_t k=0x40133333;
 // Keep the second subtraction/add in x87 until multiplication and summation.
 if(y) __asm__ volatile("flds %1; fsubs %2; fadds %3; fmuls %4; fadds %5; fstps %0":"=m"(out):"m"(origin),"m"(position),"m"(k),"m"(previousDifference),"m"(sum):"st");
 else if(add) __asm__ volatile("flds %1; fsubs %2; fmuls %3; fadds %4; fstps %0":"=m"(out):"m"(origin),"m"(position),"m"(previousDifference),"m"(sum):"st");
 else __asm__ volatile("flds %1; fsubs %2; fmuls %3; fstps %0":"=m"(out):"m"(origin),"m"(position),"m"(previousDifference):"st");
 return out;
}
uint32_t multiply(uint32_t a,uint32_t b) { uint32_t out; __asm__ volatile("flds %1; fmuls %2; fstps %0":"=m"(out):"m"(a),"m"(b):"st"); return out; }
bool greater(const void* a,const void* b) { uint16_t status; __asm__ volatile("flds (%2); flds (%1); fcompp; fnstsw %%ax":"=a"(status):"r"(a),"r"(b):"st"); return (status&0x4500)==0; }
}
uint32_t run(State s,const Dependencies& d) {
 Vector origin=scratch(),reference=scratch(); d.init(&origin,0,0,0x41200000);
 const auto z=bits(d.access(s.at(0x184c98),2)); const auto y=bits(d.access(s.at(0x184c98),1)); const auto x=bits(d.access(s.at(0x184c98),0));
 d.init(&reference,x,y,z); d.normalize(&reference);
 for(int32_t i=0;i<integer(s.at(0x177f88));++i) {
  auto* r=static_cast<uint8_t*>(s.at(0x178110))+i*56;
  uint32_t first=bits(d.access(&origin,0)); auto diff=difference(first,bits(r),false);
  first=bits(d.access(&origin,0)); auto sum=contribution(first,bits(r),false,diff,0,false);
  first=bits(d.access(&origin,1)); diff=difference(first,bits(r+4),true);
  first=bits(d.access(&origin,1)); sum=contribution(first,bits(r+4),true,diff,sum,true);
  first=bits(d.access(&origin,2)); diff=difference(first,bits(r+8),false);
  first=bits(d.access(&origin,2)); put(r+44,contribution(first,bits(r+8),false,diff,sum,true));
  Vector direction=scratch();
  first=bits(d.access(&origin,2)); const auto dz=difference(first,bits(r+8),false,true);
  first=bits(d.access(&origin,1)); const auto dy=difference(first,bits(r+4),true,true);
  first=bits(d.access(&origin,0)); const auto dx=difference(first,bits(r),false,true);
  d.init(&direction,dx,dy,dz); d.normalize(&direction);
  put(r+48,selection_score(d.angular,&reference,&direction,d.magnitude));
  put(r+52,multiply(bits(r+44),bits(r+48)));
 }
 const auto rotation=[&](unsigned index,uint64_t coefficient) {
  Vector source=scratch(),output=scratch(); d.init(&source,0,0xbf800000,0);
  const uint32_t angleBits=selection_angle(d.sine,bits(s.at(0x184c90)),coefficient); float angle; std::memcpy(&angle,&angleBits,4);
  const void* returned=d.rotate(&output,source,angle,1.0f,0.0f,0.0f); std::memcpy(&source,returned,16);
  for(int32_t axis=0;axis<3;++axis) { const auto value=bits(d.access(&source,axis)); put(static_cast<uint8_t*>(s.at(0x178110))+index*56+24+axis*4,value); }
 };
 if(integer(s.at(0x184770))==1) { rotation(0,0x3fc3333340000000ULL); rotation(1,0xbfc3333340000000ULL); }
 if(integer(s.at(0x184770))==2) rotation(0,0x3fc3333340000000ULL);
 if(observer)observer(observerContext,0,s);
 for(int32_t i=0;i<integer(s.at(0x177f88))-1;++i) for(int32_t j=i+1;j<integer(s.at(0x177f88));++j) {
  auto* a=static_cast<uint8_t*>(s.at(0x178110))+i*56; auto* b=static_cast<uint8_t*>(s.at(0x178110))+j*56;
  if(greater(a+44,b+44)) { uint32_t saved[14]; std::memcpy(saved,a,56); std::memcpy(a,b,56); saved[9]=7; std::memcpy(b,saved,56); }
 }
 if(observer)observer(observerContext,1,s);
 d.wave();
 if(observer)observer(observerContext,2,s);
 for(unsigned k=0;k<3;++k) {
  auto* r=static_cast<uint8_t*>(s.at(0x178110))+k*56;
  for(unsigned c=0;c<3;++c) put(s.at(positions[k]+c*4),bits(r+c*4));
  for(unsigned c=0;c<3;++c) put(s.at(colors[k]+c*4),0x3f800000);
  for(unsigned c=0;c<3;++c) put(s.at(directions[k]+c*4),bits(r+24+c*4));
  put(s.at(parameters[k]),bits(r+36));
 }
 uint32_t result=0;
 for(unsigned call=0;call<7;++call) {
  if(call>=1 && call<=3) {
   const unsigned k=call-1; const auto b=bits(s.at(colors[k]+8)),g=bits(s.at(colors[k]+4)),r=bits(s.at(colors[k])); const auto handle=bits(s.at(handles[call]));
   result=reinterpret_cast<Color>(bits(s.at(0x1855cc)))(handle,r,g,b,0x3f800000);
   const uint32_t args[]={handle,r,g,b,0x3f800000}; if(s.trace) s.trace(s.context,call,args,result);
  } else { const auto parameter=call==0?0:bits(s.at(parameters[call-4])); const auto handle=bits(s.at(handles[call])); result=reinterpret_cast<Scalar>(bits(s.at(0x1855dc)))(handle,parameter); const uint32_t args[]={handle,parameter,0,0,0}; if(s.trace) s.trace(s.context,call,args,result); }
 }
 return result;
}
Dependencies dependencies(uintptr_t b) { return {reinterpret_cast<Init>(b+0x1839),reinterpret_cast<Access>(b+0x19c9),reinterpret_cast<Normalize>(b+0x1811),reinterpret_cast<Angular>(b+0x17f8),reinterpret_cast<Magnitude>(b+0x11e0),reinterpret_cast<Rotate>(b+0x101e),reinterpret_cast<Sin>(b+0x95c34),reinterpret_cast<Wave>(b+0x1393)}; }
}

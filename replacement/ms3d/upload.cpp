#include "upload.hpp"
namespace upload {
uint32_t run(const char* filename,const Callbacks& c) {
    volatile uint32_t id=0;
    Observation observed{}; observed.filename=filename;
    Image* image=c.decode(filename);
    observed.image=image!=nullptr;
    if(!image) { if(c.observe) c.observe(observed); return 0; }
    auto* live=static_cast<volatile Image*>(image);
    observed.pixels=live->pixels!=nullptr;
    if(!observed.pixels) { if(c.observe) c.observe(observed); return 0; }
    c.generate(1,const_cast<uint32_t*>(&id)); observed.generated=id;
    c.bind(0x0de1,id);
    c.parameter(0x0de1,0x2801,0x2703);
    c.parameter(0x0de1,0x2800,0x2601);
    const void* pixels=live->pixels;
    const int32_t height=live->height;
    const int32_t width=live->width;
    observed.width=width; observed.height=height;
    observed.status=c.mipmap(0x0de1,3,width,height,0x1907,0x1401,pixels); observed.mipmap=true;
    void* releasedPixels=live->pixels;
    c.release(releasedPixels); observed.pixelReleased=true;
    c.release(image); observed.recordReleased=true;
    observed.result=id;
    if(c.observe) c.observe(observed);
    return id;
}
}

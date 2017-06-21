#ifndef _COLOR_H_
#define _COLOR_H_

#define ALIGN 32

#ifdef __INTEL_COMPILER
struct __declspec(align(32))  Color
#else
struct Color
#endif
{
    float r, g, b, a;
};

#endif



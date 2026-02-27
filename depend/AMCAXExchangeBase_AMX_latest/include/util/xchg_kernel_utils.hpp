#ifndef __XCHG_KERNEL_UTILS_HPP__
#define __XCHG_KERNEL_UTILS_HPP__

#include "base/xchg_export.hpp"
#include "base/xchg_transfo.hpp"
#include "topology/xchg_body.hpp"

extern "C"
{
    extern "C" void FSTART(int*);
    extern "C" void FABORT(int*);
    extern "C" void FSTOP(int*);
    extern "C" void FMALLO(int*, char**, int*);
    extern "C" void FMFREE(int*, char**, int*);
    extern "C" void GOSGMT(const int*, const int*, const int*, const int*, const double*, const int*, const int*, int*);
    extern "C" void GOOPSG(const int*, const int*, const int*, const int*, const double*, const int*, const int*, int*);
    extern "C" void GOCLSG(const int*, const int*, const int*, const int*, const double*, const int*, const int*, int*);
    extern "C" void GOPIXL(const int*, const double*, const int*, const int*, int*);
    extern "C" void GOOPPX(const int*, const double*, const int*, const int*, int*);
    extern "C" void GOCLPX(const int*, const double*, const int*, const int*, int*);
    extern "C" void FFOPRD(const int*, const int*, const char*, const int*, const int*, int*, int*);
    extern "C" void FFOPWR(const int*, const int*, const char*, const int*, const char*, const int*, int*, int*);
    extern "C" void FFCLOS(const int*, const int*, const int*, int*);
    extern "C" void FFREAD(const int*, const int*, const int*, char*, int*, int*);
    extern "C" void FFWRIT(const int*, const int*, const int*, const char*, int*);
    extern "C" void FFOPRB(const int*, const int*, const int*, int*, int*, int*);
    extern "C" void FFSEEK(const int*, const int*, const int*, int*);
    extern "C" void FFTELL(const int*, const int*, int*, int*);
    extern "C" void FGCRCU(const char*, int*, int*, int*, int*, double*, int*, double*, int*);
    extern "C" void FGCRSU(const char*, int*, int*, int*, int*, double*, int*, double*, int*);
    extern "C" void FGEVCU(int*, double*, double*, double*, int*, double*, int*);
    extern "C" void FGEVSU(int*, double*, double*, double*, double*, int*, int*, int*, double*, int*);
    extern "C" void FGPRCU(int*, double*, double*, double*, int*, int*);
    extern "C" void FGPRSU(int*, double*, double*, double*, int*, int*);
}

XCHG_API Xchg_Int32 StartSession();
XCHG_API Xchg_Int32 StopSession();

Xchg_Int32 convertToPKBody(const Xchg_BodyPtr& inBody, Xchg_Int32& outBodyTag);
Xchg_Int32 convertToPKTransf(const Xchg_transfo& inTransfo, Xchg_Int32& outTransf);

#endif

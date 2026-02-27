#ifndef __XCHG_STATUS_HPP__
#define __XCHG_STATUS_HPP__
#include "base/xchg_export.hpp"
#include <fstream>
#include <math.h>
#include "base/xchg_type.hpp"
#include "base/xchg_vector.hpp"

enum Xchg_status_enum {
    XCHG_ISERROR = -1,
        XCHG_NOERROR = 0
};
//! \defgroup base_types Base type Classes 
//! Gathers all base type classes - string, matrix etc...-

class XCHG_API Xchg_status
{
private:
    static Xchg_status _xchg_default_status;
    Xchg_status_enum             _st;
    void init();
    void reset();
    void copy(const Xchg_status& s);
public:
    Xchg_status();
    Xchg_status(const Xchg_status& s);
    ~Xchg_status();

    Xchg_status(const Xchg_status_enum &s);
    //operator Xchg_status_enum();
    Xchg_status& operator = (const Xchg_status& s);
    friend Xchg_bool operator == (const Xchg_status &s1, const Xchg_status &s2) ;
    static Xchg_status& GetDefaultStatus();
    Xchg_bool isOK() const;
    Xchg_bool isNotOK() const;
    void setOK();
    void setNotOK();
};
#endif

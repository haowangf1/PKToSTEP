#ifndef __XCHG_RGB_HPP__
#define __XCHG_RGB_HPP__
#include "base/xchg_export.hpp"
#include <fstream>
#include "base/xchg_type.hpp"

class XCHG_API Xchg_RGB
{
private:
    int _RGBA[5];
    void init();
    void reset();
    bool compare(Xchg_RGB) const;
    void copy(const Xchg_RGB& s);
public:
    Xchg_RGB();
    Xchg_RGB(const Xchg_RGB& s);
    Xchg_RGB(int r, int g, int b, int a = -1, int inIndexReader = -1 );
    Xchg_RGB(int (&rgba)[5]);
    Xchg_RGB(int (&rgba)[4]);
    Xchg_RGB(int (&rgb)[3]);
	void Clear() { reset(); }
    ~Xchg_RGB();

    Xchg_RGB& operator = (const Xchg_RGB& s);
    int& operator [](int pos);
    const int& operator [](int pos) const;
    
	inline bool HasColor() const { return _RGBA[0] >= 0;}
    
	inline bool HasAlpha() const { return _RGBA[3] >= 0;}
    
	inline bool IsFilled() const { return (_RGBA[0] >= 0 || _RGBA[3] >= 0);}

    inline		 int & R()		 {return _RGBA[0];}
	inline const int & R() const {return _RGBA[0];}

    inline		 int & G()		 {return _RGBA[1];}
	inline const int & G() const {return _RGBA[1];}

    inline		 int & B()		 {return _RGBA[2];}
	inline const int & B() const {return _RGBA[2];}

    inline		 int & A()		 {return _RGBA[3];}
	inline const int & A() const {return _RGBA[3];}
    
	inline		 int & IndexReader()		 {return _RGBA[4];}
	inline const int & IndexReader() const {return _RGBA[4];}
    
    inline void SetRGBA(const int& inRed, const int& inGreen, const int& inBlue, const int& inAlpha = -1)
    {
        _RGBA[0] = inRed;
        _RGBA[1] = inGreen;
        _RGBA[2] = inBlue;
        _RGBA[3] = inAlpha;
    }

    inline void GetRGB(int& outRed, int& outGreen, int& outBlue)
    {
        outRed      = _RGBA[0];
        outGreen    = _RGBA[1];
        outBlue     = _RGBA[2];
    }
    inline void GetRGBA(int& outRed, int& outGreen, int& outBlue, int& outAlpha)
    {
        GetRGB(outRed, outGreen, outBlue);
        outAlpha    = _RGBA[3];
    }

	friend bool operator==(const Xchg_RGB & lhs, const Xchg_RGB & rhs);

	friend bool operator<(const Xchg_RGB & lhs, const Xchg_RGB & rhs);

	friend bool operator!=(const Xchg_RGB & lhs, const Xchg_RGB & rhs);

	friend bool operator>(const Xchg_RGB & lhs, const Xchg_RGB & rhs);

	friend bool operator<=(const Xchg_RGB & lhs, const Xchg_RGB & rhs);

	friend bool operator>=(const Xchg_RGB & lhs, const Xchg_RGB & rhs);
};


#endif

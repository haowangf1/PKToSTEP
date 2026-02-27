#ifndef __XCHG_OBJECT_HPP__
#define __XCHG_OBJECT_HPP__
#include "base/xchg_export.hpp"
#include "base/xchg_type.hpp"
#include <iosfwd>


// Forward declarations
template <typename T> class SmartPtr;

/*
统一基类:       为所有Exchange数据模型对象提供统一的接口和行为
内存管理:       通过智能指针实现自动内存管理，避免内存泄漏
类型安全:       提供运行时类型识别，确保安全的对象转换
对象生命周期:   标准化对象初始化、复制和销毁过程
可扩展性:       为派生类提供基础功能，支持数据模型的扩展和兼容性
*/


class XCHG_API Xchg_Object
{
protected:
    void _init();
    void _reset();
    void _copy(const Xchg_Object& s);
    enum { _typeID = XCHG_TYPE_OBJECT }; 
    unsigned long count_;
    
    // Allow all SmartPtr instantiations to access count_
    template <typename T> friend class SmartPtr;
    friend class SmartPtr<Xchg_Object>;
public:
    Xchg_Object();
    Xchg_Object(const Xchg_Object& s);
    virtual ~Xchg_Object();
    Xchg_Object& operator = (const Xchg_Object& s);

    virtual void dump(FILE * file = stdout);
    
    // Downcasting - Runtime Type Identification System
    // Each derived class should implement these methods following this pattern:
    // 1. Define unique _typeID in protected section
    // 2. Implement DtkDynamicType to check type hierarchy
    // 3. Implement static DtkDynamicCast for safe type conversion
    virtual Xchg_Int32 DtkDynamicType(const Xchg_Int32& inId);
    static Xchg_Object* DtkDynamicCast(Xchg_Object *s);
    
    // Clone method for deep copying (used by SmartPtr::Clone)
    // Default implementation returns nullptr - override in derived classes if needed
    virtual Xchg_Object* Clone() const { return nullptr; }
    
    friend std::ostream& operator<<(std::ostream& o,const Xchg_Object& d);
    virtual Size_t GetSize() const;
};

#endif

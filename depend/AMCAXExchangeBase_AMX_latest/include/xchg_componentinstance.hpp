#ifndef COMPONENT_INSTANCE_H
#define COMPONENT_INSTANCE_H

#include <string>

#include "base/xchg_export.hpp"
#include "base/xchg_objectbase.hpp"
#include "base/xchg_smartptr.hpp"
#include "base/xchg_string.hpp"
#include "base/xchg_transfo.hpp"
#include "base/xchg_type.hpp"
#include "xchg_docelement.hpp"

// 前置声明 Xchg_Component - 避免循环包含 Component.hpp
class Xchg_Component;
typedef SmartPtr<Xchg_Component> Xchg_ComponentPtr;

class Xchg_ComponentInstance;
typedef SmartPtr<Xchg_ComponentInstance> Xchg_ComponentInstancePtr;

/// @brief
class XCHG_API Xchg_ComponentInstance : public Xchg_DocElement
{
protected:
    enum { _typeID = XCHG_TYPE_INSTANCE };

private:
    void _Init();
    void _Reset();
    void _Copy(const Xchg_ComponentInstance& s);

    //! Copy Constructor
    Xchg_ComponentInstance(const Xchg_ComponentInstance& other);

    ~Xchg_ComponentInstance();
    friend class SmartPtr<Xchg_ComponentInstance>;

public:
    Xchg_ComponentInstance(Xchg_string inComponentInstanceName,
                           const Xchg_transfo& inTransformationMatrix,
                           const Xchg_ComponentID& inID);

    Xchg_ComponentPtr GetComponent() const;
    void SetComponent(Xchg_ComponentPtr comp);

    //! \DtkDynamicType
    inline Xchg_Int32 DtkDynamicType(const Xchg_Int32& inId)
    {
        if (inId == _typeID)
            return 1;
        return Xchg_DocElement::DtkDynamicType(inId);
    }

    //! \DtkDynamicCast
    inline static Xchg_ComponentInstance* DtkDynamicCast(Xchg_Object* s)
    {
        if (s && s->DtkDynamicType(_typeID))
            return static_cast<Xchg_ComponentInstance*>(s);
        return NULL;
    }

    //! \Clone
    inline virtual Xchg_Object* Clone() const override
    {
        return new Xchg_ComponentInstance(*this);
    }

    Xchg_transfo GetTransform() const;
    void SetTransform(const Xchg_transfo& matrix);

    std::string GetName() const;
    void SetName(const std::string& name);

    Xchg_Int32 GetKernelTransf(Xchg_Int32& outTransf) const;

private:
    /// The name of the document instance
    Xchg_string _name;

    /// The document reference of the instance
    Xchg_ComponentPtr _component;

    /// The transformation matrix of the instance
    Xchg_transfo _transformationMatrix;
};

#endif  // COMPONENT_INSTANCE_H

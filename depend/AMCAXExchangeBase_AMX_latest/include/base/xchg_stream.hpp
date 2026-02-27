#ifndef __UTIL_XCHG_STREAM_HPP__
#define __UTIL_XCHG_STREAM_HPP__

#include "base/xchg_export.hpp"
#include "base/xchg_type.hpp"
#include "base/xchg_objectbase.hpp"
#include "base/xchg_string.hpp"
#include "base/xchg_smartptr.hpp"
#include <vector>
#include <memory>


//! \class Xchg_Stream 
//! \brief This is the Xchg_Stream Class.
//!
//! The Xchg_Stream object is used to store binary stream data for embedded files
//! \SmartPtr{Xchg_StreamPtr}

class XCHG_API Xchg_Stream : public Xchg_Object
{
public:
	enum Xchg_StreamType
	{
		XCHG_STREAM_UNKNOWN,
		XCHG_STREAM_X_B,
		XCHG_STREAM_JT,
		XCHG_STREAM_XMM_B,
	};

protected:
	enum { _typeID = XCHG_TYPE_STREAM };
	
	// Direct data members (no Pimpl)
	Xchg_string _name;                     // Stream name
	std::vector<Xchg_Char8> _binary_data;  // Binary content
	Xchg_StreamType _type;                 // Stream type
	
	void _Init();
	void _Copy(const Xchg_Stream &o);
	void _Reset();
	
	//! \brief Base Constructor (protected)
	Xchg_Stream(const Xchg_string &inName, const Xchg_Char8 *const inBinaryContent,
		const Xchg_Size_t& inBinaryContentSize, Xchg_StreamType inType);
	
	//! \brief Copy Constructor (protected)
	Xchg_Stream(const Xchg_Stream &o);
	
	//! \brief Move Constructor (protected)
	Xchg_Stream(Xchg_Stream&& o) XCHG_NOEXCEPT;
	
	friend class SmartPtr<Xchg_Stream>;
	//! \brief Destructor
	~Xchg_Stream();

public:
	// Static factory methods
	//! \brief Create a Xchg_StreamPtr from binary data
	//! \param inName Stream name
	//! \param inBinaryContent Binary data pointer
	//! \param inBinaryContentSize Data size
	//! \param inType Stream type
	//! \return SmartPtr to new Xchg_Stream
	static SmartPtr<Xchg_Stream> Create(
		const Xchg_string &inName, 
		const Xchg_Char8 *const inBinaryContent,
		const Xchg_Size_t& inBinaryContentSize, 
		Xchg_StreamType inType);

	//! \brief Create a Xchg_StreamPtr from copy
	//! \param inToBeCopied Stream to copy
	//! \return SmartPtr to new Xchg_Stream
	static SmartPtr<Xchg_Stream> Create(const Xchg_Stream &inToBeCopied);

	virtual Xchg_Object* Clone() const override { return new Xchg_Stream(*this); }

	
	// RTTI methods
	//! \brief Dynamic type checking
	inline Xchg_Int32 DtkDynamicType(const Xchg_Int32& inId)
	{
		if (inId == _typeID) return 1;
		return Xchg_Object::DtkDynamicType(inId);
	}
	
	//! \brief Dynamic cast
	inline static Xchg_Stream* DtkDynamicCast(Xchg_Object* s)
	{
		if (s && s->DtkDynamicType(_typeID))
			return static_cast<Xchg_Stream*>(s);
		return NULL;
	}

	// Accessors
	//! \brief Returns the stream name
	Xchg_string GetName() const;
	
	//! \brief Returns the binary content size
	Xchg_Size_t GetBinaryContentSize() const;
	
	//! \brief Returns the binary content pointer
	const Xchg_Char8* GetBinaryContent() const;
	
	//! \brief Returns the stream type
	Xchg_StreamType GetStreamType() const;

	// Modifiers
	//! \brief Set the binary content
	//! \param inNewContent New binary data
	//! \param inNewContentSize Size of new data
	void SetBinaryContent(
		const Xchg_Char8 *const inNewContent,
		const Xchg_Size_t inNewContentSize);
};

// Type alias for SmartPtr
typedef SmartPtr<Xchg_Stream> Xchg_StreamPtr;


#endif // __UTIL_XCHG_STREAM_HPP__

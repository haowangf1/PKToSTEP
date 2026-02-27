#ifndef __UTIL_XCHG_LINE_TYPE_DEFINITION_HPP__
#define __UTIL_XCHG_LINE_TYPE_DEFINITION_HPP__
#include "base/xchg_export.hpp"
#include "base/xchg_objectbase.hpp"
#include "base/xchg_string.hpp"
#include "base/xchg_type.hpp"
#include "base/xchg_entity.hpp"
#include "base/xchg_smartptr.hpp"

class Xchg_LineTypeDefinition;

typedef SmartPtr<Xchg_LineTypeDefinition> Xchg_LineTypeDefinitionPtr;


/////////////////////////////////////////////////////////////////////////////

//! \ingroup DtkAPIGroup
//! \class Xchg_LineTypeDefinition 
//! \brief This is the Xchg_LineTypeDefinition Class.\n
//! The Xchg_LineTypeDefinition object is used to store any informations about style of curves.
//! \SmartPtr{Xchg_LineTypeDefinitionPtr}

class XCHG_API Xchg_LineTypeDefinition : public Xchg_Object
{
public:
	enum Xchg_LineTypePatternDescriptorType
	{
		XCHG_LINE_TYPE_PATTERN_DESCRIPTOR_UNKNOWN,
		XCHG_LINE_TYPE_PATTERN_DESCRIPTOR_DASH,
		XCHG_LINE_TYPE_PATTERN_DESCRIPTOR_DOT,
		XCHG_LINE_TYPE_PATTERN_DESCRIPTOR_GAP,
	};

protected:
	enum
	{
		_typeID = XCHG_TYPE_LINE_TYPE_DEFINITION
	};
	struct Xchg_Handle;
	Xchg_Handle *_Private = nullptr;

	//! \CopyConstructor{inToBeCopied}
	Xchg_LineTypeDefinition( const Xchg_LineTypeDefinition & inToBeCopied );

	Xchg_LineTypeDefinition( const Xchg_string & inName );

	Xchg_LineTypeDefinition( const Xchg_string & inName,
							const bool inHasBoldSegmentsOnChangeOfDirections,
							const Xchg_Double64 & inBoldSegmentsOnChangeOfDirectionsThickness,
							const Xchg_Double64 & inBoldSegmentsOnChangeOfDirectionsLength );

	Xchg_LineTypeDefinition( const Xchg_FontLineType & inClosestAppearanceInFontLineTypeEnumeration );

	//! \BaseDestructor 
	virtual ~Xchg_LineTypeDefinition();
	friend class SmartPtr<Xchg_LineTypeDefinition>;

	void _Init();
	void _Copy( const Xchg_LineTypeDefinition & inToBeCopied );
	void _Reset();
	Xchg_Object* Clone() const override
	{
		return new Xchg_LineTypeDefinition( *this );
	}

public:
	//! \brief Create a Xchg_LineTypeDefinitionPtr.
	//! \param inName Name of the Xchg_LineTypeDefinition to construct.
	//! \return The constructed Xchg_LineTypeDefinitionPtr.
	static Xchg_LineTypeDefinitionPtr Create( const Xchg_string & inName );

	//! \brief Create a Xchg_LineTypeDefinitionPtr.
	//! \param inName Name of the Xchg_LineTypeDefinition to construct.
	//! \param inHasBoldSegmentsAtEndOfLines If true, the curves have a specific thickness when there is a change of direction.
	//! \param inBoldSegmentsOnEndOfLinesThickness Precise the specific thickness if inHasBoldSegmentsAtEndOfLines is true.
	//! \param inBoldSegmentsOnEndOfLinesLength Precise the specific thickness length if inHasBoldSegmentsAtEndOfLines is true.
	//! \return The constructed Xchg_LineTypeDefinitionPtr.
	static Xchg_LineTypeDefinitionPtr Create( const Xchg_string & inName,
											 const bool inHasBoldSegmentsAtEndOfLines,
											 const Xchg_Double64 & inBoldSegmentsOnEndOfLinesThickness,
											 const Xchg_Double64 & inBoldSegmentsOnEndOfLinesLength );

	//! \brief Create a Xchg_LineTypeDefinitionPtr without any pattern but only a closest enumeration value.
	//! \return The constructed Xchg_LineTypeDefinitionPtr.
	static Xchg_LineTypeDefinitionPtr Create( const Xchg_FontLineType & inClosestAppearanceInFontLineTypeEnumeration );

	//! \DtkDynamicType
	inline Xchg_Int32 DtkDynamicType(const Xchg_Int32& inId)
	{
		if (inId == _typeID) return 1;
		return Xchg_Object::DtkDynamicType(inId);
	}

	//! \DtkDynamicCast
	inline static Xchg_LineTypeDefinition* DtkDynamicCast(Xchg_Object* s)
	{
		if (s && s->DtkDynamicType(_typeID))
			return static_cast<Xchg_LineTypeDefinition*>(s);
		return NULL;
	}

public:
	//! \brief Returns the line type definition name.
	const Xchg_string & GetName() const;

	//! \brief Set the line type definition name.
	void SetName( const Xchg_string& inName );

	//! \brief Get what we consider as the closest type in the Xchg_FontLineType enumeration.
	//! \attention We recommend not using this function if the integration uses the patterns. As it is not precise, it should only be used for fast integrations that don't uses patterns.
	//! \returns Xchg_FontLineType::XCHG_NO_PATTERN if no enumeration value is considered close.
	Xchg_FontLineType GetClosestAppearanceInFontLineTypeEnumeration() const;

	//! \brief Set what can be consider as the closest type in the Xchg_FontLineType enumeration.
	void SetClosestAppearanceInFontLineTypeEnumeration( const Xchg_FontLineType & inClosestFontLineTypeEnumValue ) const;

	//! \brief Get the the number of pattern descriptor.
	Xchg_Size_t GetPatternLength() const;

	//! \brief Get the type of the pattern descriptor at index to call the correct function.
	//! \return dtkNoError if OK. dtkErrorOutOfRange if index is out of bounds.
	Xchg_ErrorStatus GetPatternDescriptorTypeAtIndex( const Xchg_Size_t & inIndex,
													 Xchg_LineTypePatternDescriptorType & outPatternType ) const;

	//! \brief Get the length of the segment at index in the pattern.
	//! \return dtkNoError if OK. dtkErrorOutOfRange if index is out of bounds or not of valid type.
	Xchg_ErrorStatus GetPatternDescriptorDashAtIndex( const Xchg_Size_t & inIndex,
													 Xchg_Double64 & outDashLength ) const;

	//! \brief Add a new pattern descriptor of type DASH and precise it's length.
	void PushBackPatternDescriptorDash( const Xchg_Double64 & inDashLength ) const;

	//! \brief Add a new pattern descriptor of type DOT.
	void PushBackPatternDescriptorDot() const;

	//! \brief Get the length of the gap at index in the pattern.
	//! \return dtkNoError if OK. dtkErrorOutOfRange if index is out of bounds or not of valid type.
	Xchg_ErrorStatus GetPatternDescriptorGapAtIndex( const Xchg_Size_t & inIndex,
													Xchg_Double64 & outGapLength ) const;

	//! \brief Add a new pattern descriptor of type GAP and precise it's length.
	void PushBackPatternDescriptorGap( const Xchg_Double64 & inGapLength ) const;

	//! \brief Get bold segments when curve changes of direction.
	//! \return True if the style has bold segments on change of directions, False if doesn't have those bold segments.
	bool GetBoldSegmentsOnChangeOfDirections( Xchg_Double64 & outBoldSegmentsThickness, Xchg_Double64 & outBoldSegmentsLength ) const;
};


#endif // __UTIL_XCHG_LINE_TYPE_DEFINITION_HPP__

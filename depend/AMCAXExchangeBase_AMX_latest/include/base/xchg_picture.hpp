
#ifndef __XCHG_PICTURE_HPP__
#define __XCHG_PICTURE_HPP__

#include "base/xchg_export.hpp"
#include "base/xchg_type.hpp"
#include "base/xchg_objectbase.hpp"
#include "base/xchg_transfo.hpp"
#include "base/xchg_pnt.hpp"
#include "base/xchg_smartptr.hpp"
#include "base/xchg_vector.hpp"
#include "base/xchg_entity.hpp"

//! \ingroup base_types
//! \class Xchg_picture
//! \brief This class defines a picture
//!
//!    This class defines a picture for embedding images in CAD files
class Xchg_Info;


// Forward declarations
enum Xchg_PictureType
{
	XCHG_PICTURE_UNKNOWN,
    XCHG_PICTURE_RGB24,
    XCHG_PICTURE_RGBA32,
	XCHG_PICTURE_JPG,
	XCHG_PICTURE_BMP,
	XCHG_PICTURE_PNG,
	XCHG_PICTURE_CGM,
    XCHG_PICTURE_GIF,
    XCHG_PICTURE_TIFF,
    XCHG_PICTURE_ICO,
    XCHG_PICTURE_PS,
    XCHG_PICTURE_SVG,
    XCHG_PICTURE_OLE,
	XCHG_PICTURE_WMF,
	XCHG_PICTURE_EMF,
	XCHG_PICTURE_TGA
};
class XCHG_API Xchg_picture : public Xchg_Object
{
protected:
    enum {
        _typeID = XCHG_TYPE_PICTURE
    };
    
    Xchg_InfoPtr _Info;                // 附加元数据
    Xchg_vector<char> _file;               // 二进制图片数据
    Xchg_PictureType _type;             // 图片类型
    Xchg_Size_t _pixel_width;           // 像素宽度
    Xchg_Size_t _pixel_height;          // 像素高度
    Double64 _scale_x;                 // X轴缩放因子
    Double64 _scale_y;                 // Y轴缩放因子
    Double64 _metric_width;            // 物理宽度（毫米）
    Double64 _metric_height;           // 物理高度（毫米）
    Xchg_pnt _origin;                   // 原点位置

    void _Init();
    void _Copy(const Xchg_picture &inToBeCopied);
    void _Reset();
    
    friend class SmartPtr<Xchg_picture>;
public:
    // Static factory methods
    static SmartPtr<Xchg_picture> Create();
    static SmartPtr<Xchg_picture> Create(const Xchg_picture &inToBeCopied);
    virtual Xchg_Object* Clone() const override { return new Xchg_picture(*this); }

    // Constructors & Destructor
    Xchg_picture();
    Xchg_picture(const Xchg_picture &inToBeCopied);
    Xchg_picture(Xchg_picture&& other) XCHG_NOEXCEPT;
    ~Xchg_picture();
    
    // Assignment operators
    Xchg_picture& operator=(const Xchg_picture &inToBeCopied);
    Xchg_picture& operator=(Xchg_picture&& other) XCHG_NOEXCEPT;

    // Downcasting (RTTI)
    inline Xchg_Int32 DtkDynamicType(const Xchg_Int32& inId)
    {
        if (inId == _typeID) return 1;
        return Xchg_Object::DtkDynamicType(inId);
    }
    inline static Xchg_picture* DtkDynamicCast(Xchg_Object* s)
    {
        if (s && s->DtkDynamicType(_typeID))
            return static_cast<Xchg_picture*>(s);
        return NULL;
    }

    // Info accessors
    Xchg_InfoPtr GetInfo() const;
    Xchg_InfoPtr& GetInfo();

    //! \brief Retrieves the binary file
    //! \return the binary file
    inline Xchg_vector<char> & File() { return this->_file; }
    inline const Xchg_vector<char> & File() const { return this->_file; }
    //! \brief Retrieves the file type
    //! \return the absolute file type
    inline Xchg_PictureType & FileType() { return this->_type; }
    inline Xchg_PictureType FileType() const { return this->_type; }
    //! \brief Retrieves the width (in pixels)
    //! \return the width (in pixels)
    inline Xchg_Size_t &PixelWidth() { return this->_pixel_width; }
    inline Xchg_Size_t PixelWidth() const { return this->_pixel_width; }

    //! \brief Retrieves the height (in pixels)
    //! \return the height (in pixels)
    inline Xchg_Size_t &PixelHeight() { return this->_pixel_height; }
    inline Xchg_Size_t PixelHeight() const { return this->_pixel_height; }

    //! \brief Retrieves the width (in mm)
    //! \return the width (in mm)
    inline Double64& MetricWidth() { return this->_metric_width; }
    inline Double64 MetricWidth() const { return this->_metric_width; }
    //! \brief Retrieves the height (in mm)
    //! \return the height (in mm)
    inline Double64& MetricHeight() { return this->_metric_height; }
    inline Double64 MetricHeight() const { return this->_metric_height; }
    //! \brief Retrieves the origin (in mm)
    //! \return the origin (in mm)
    inline Xchg_pnt& Origin() { return this->_origin; }
    inline const Xchg_pnt& Origin() const { return this->_origin; }

    //! \brief Retrieves the X scale factor
    //! \return the X scale factor
    inline Double64& ScaleX() { return this->_scale_x; }
    inline Double64 ScaleX() const { return this->_scale_x; }
    //! \brief Retrieves the Y scale factor
    //! \return the Y scale factor
    inline Double64& ScaleY() { return this->_scale_y; }
    inline Double64 ScaleY() const { return this->_scale_y; }
    
    //! \brief Copy from pointer
    void _Copy(const Xchg_picture* s);

    //! \brief Apply transformation
    //! \param inTransfo the transformation to apply
    //! \return error status
    Xchg_ErrorStatus Transform(const Xchg_transfo& inTransfo);
};


#endif // __XCHG_PICTURE_HPP__

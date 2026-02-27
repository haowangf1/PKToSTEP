#ifndef __UTIL_XCHG_DOCUMENT_INFORMATION_HPP__
#define __UTIL_XCHG_DOCUMENT_INFORMATION_HPP__

#include "base/xchg_export.hpp"
#include "base/xchg_type.hpp"
#include "base/xchg_string.hpp"

/////////////////////////////////////////////////////////////////////////////
//! \ingroup DtkAPIGroup
//! \class Xchg_DocumentInformation 
//! \brief This class holds document metadata information
//! It contains information about the file, author, organization, version, etc.
class XCHG_API Xchg_DocumentInformation
{
public:
    //! \brief Constructor
    Xchg_DocumentInformation();
    
    //! \brief Copy constructor
    Xchg_DocumentInformation(const Xchg_DocumentInformation& other);
    
    //! \brief Assignment operator
    Xchg_DocumentInformation& operator=(const Xchg_DocumentInformation& other);
    
    //! \brief Destructor
    ~Xchg_DocumentInformation();

    // Getters
    const Xchg_string& GetFileName() const { return _fileName; }
    const Xchg_string& GetAuthor() const { return _author; }
    const Xchg_string& GetOrganization() const { return _organization; }
    const Xchg_string& GetSourceSystem() const { return _sourceSystem; }
    const Xchg_string& GetVersion() const { return _version; }
    const Xchg_string& GetUnit() const { return _unit; }
    const Xchg_string& GetComment() const { return _comment; }
    Xchg_Double64 GetTolerance() const { return _tolerance; }
    const Xchg_string& GetFormatType() const { return _formatType; }
    const Xchg_string& GetDocumentType() const { return _documentType; }

    // Setters
    void SetFileName(const Xchg_string& inFileName) { _fileName = inFileName; }
    void SetAuthor(const Xchg_string& inAuthor) { _author = inAuthor; }
    void SetOrganization(const Xchg_string& inOrganization) { _organization = inOrganization; }
    void SetSourceSystem(const Xchg_string& inSourceSystem) { _sourceSystem = inSourceSystem; }
    void SetVersion(const Xchg_string& inVersion) { _version = inVersion; }
    void SetUnit(const Xchg_string& inUnit) { _unit = inUnit; }
    void SetComment(const Xchg_string& inComment) { _comment = inComment; }
    void SetTolerance(Xchg_Double64 inTolerance) { _tolerance = inTolerance; }
    void SetFormatType(const Xchg_string& inFormatType) { _formatType = inFormatType; }
    void SetDocumentType(const Xchg_string& inDocumentType) { _documentType = inDocumentType; }

    //! \brief Clear all information
    void Clear();

private:
    Xchg_string _fileName;          // File name
    Xchg_string _author;            // Author
    Xchg_string _organization;      // Organization
    Xchg_string _sourceSystem;      // Source system (e.g., "CATIA V5", "SolidWorks")
    Xchg_string _version;           // Version
    Xchg_string _unit;              // Unit (e.g., "MM", "INCH", "M")
    Xchg_string _comment;           // Comment
    Xchg_Double64 _tolerance;       // Tolerance value
    Xchg_string _formatType;        // Format type (e.g., "STEP", "IGES")
    Xchg_string _documentType;      // Document type (e.g., "Assembly", "Part")
};

#endif // __UTIL_XCHG_DOCUMENT_INFORMATION_HPP__


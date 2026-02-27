#ifndef _ERROR_XCHG_HPP_
#define _ERROR_XCHG_HPP_

enum XchgErrorStatus
{
    XCHG_OK  = 0,
    ///////////////////////////////////////////////////////////////////////////
    // Generique 
    XCHG_ERROR_INVALID_STATE = -42,
    
    XCHG_ERROR_INVALID_PARAMETER = -41,

    XCHG_ERROR_UNDEFINED = -40,

    XCHG_ERROR_NOT_FOUND = -39,

	XCHG_ERROR_FUTURE_VERSION = -38,

	XCHG_ERROR_INVALID_CLIENT_CODE                   = -37,
    XCHG_ERROR_UNAVAILABLE_WRITER                    = -36, 

	XCHG_ERROR_COMPONENT_ALREADY_LOADED			     = -35, 

	XCHG_ERROR_EMPTY_FILE			                 = -34, 

	XCHG_ERROR_CORRUPTED_ENTITY			              = -33, 

	XCHG_WARNING_NO_ENTITIES_WRITTEN			         = -32, 

	XCHG_ERROR_CORRUPTED_FILE			              = -31, 

	XCHG_ERROR_UNKNOWN_FILE_TYPE                      = -30, 

	XCHG_ERROR_UNAVAILABLE_READER                    = -29, 

	XCHG_ERROR_FILE_NOT_EXIST                         = -28, 

    XCHG_ERROR_VERSION_NOT_SUPPORTED                  = -27, 

	XCHG_ERROR_API_NOT_STARTED                        = -26, 

	XCHG_ERROR_API_ALREADY_STARTED                    = -25, 

	XCHG_ERROR_LIBRARY_VERSION                       = -24, 
    
    XCHG_ERROR_OPEN_OUTPUT_FILE                       = -23, 

    XCHG_ERROR_OPEN_INPUT_FILE                        = -22,

    XCHG_ERROR_SCHEMA_NOT_FOUND                       = -15,

    XCHG_ERROR_OPEN_CONFIG_FILE                       = -13,

	XCHG_ERROR_INPUT_FILE_SAVED_OPTION_NOT_SUPPORTED     = -7,

    XCHG_ERROR_OUTPUT_FILE                           = -6 ,

    XCHG_ERROR_INPUT_FILE                            = -5 ,

    XCHG_ERROR_LICENCE                              = -4 ,

    XCHG_ERROR_OPEN_FILES                            = -2,

    XCHG_ERROR_ALLOCATION                           = -1,

    XCHG_NO_ERROR                                   = 0 ,

    XCHG_ERROR_USER_BREAK                            = 1 ,

    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    // TOPOLOGY

    XCHG_TOPOLOGY_ERROR_UNKNOWN                      = 200,

    XCHG_ERROR_TOPOLOGY_UNREAD                       = 201,

    XCHG_WARNING_NON_MANIFOLD_TOPOLOGY                = 202,

    XCHG_WARNING_TOPOLOGY_INCOMPLETE                 = 203,

    XCHG_TOPOLOGY_LOOP_IS_VERTEX                      = 204,

    XCHG_TOPOLOGY_BODY_IS_OPEN                        = 205,

    XCHG_TOPOLOGY_BODY_IS_CLOSED                      = 206,

	XCHG_TOPOLOGY_FACE_IS_WIRE                        = 207,

	XCHG_TOPOLOGY_FACE_AS_SEVERAL_OUTER                = 208,

	XCHG_TOPOLOGY_INVALID_ID						 = 209,

	XCHG_TOPOLOGY_FACE_HAS_NO_3D_GEOM					 = 210,

	XCHG_TOPOLOGY_BODY_IS_EMPTY                      = 211,

	XCHG_TOPOLOGY_FACE_IS_UNBOUNDED                    = 212,

    XCHG_TOPOLOGY_SHELL_HAS_NO_FACES                    = 213,

	XCHG_TOPOLOGY_FACE_HAS_NO_UV_GEOM					 = 214,

    XCHG_TOPOLOGY_FACE_HAS_NO_SURFACE                  = 215,

    XCHG_WARNING_DEGENERATE_POINT                    = 216,

	XCHG_TOPOLOGY_BODY_HAS_CLOSED_AND_OPEN_SHELL         = 217,

	XCHG_TOPOLOGY_FACE_HAS_OPEN_TRIM_BOUNDARY			 = 218,

	XCHG_TOPOLOGY_BODY_HAS_SEVERAL_OPEN_SHELL           = 219,

    XCHG_TOPOLOGY_ERROR_NOT_FOUND                     = 220,

    XCHG_TOPOLOGY_ERROR_INVALID_STATE                     = 221,
    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    // Utility

    XCHG_ERROR_TYPE_NOT_SUPPORTED                     = -1000,

    XCHG_ERROR_UNSPECIFIED                          = -1001,

    XCHG_ERROR_CANNOT_COMPUTE                        = -1002,

    XCHG_ERROR_NULL_POINTER                          = -1003,

    XCHG_ERROR_ONLY_ONE_OUTER_SUPPORTED                = -1004,

    XCHG_ERROR_OUT_OF_RANGE                           = -1005,

    XCHG_ERROR_NOT_YET_IMPLEMENTED                    = -1006,

	XCHG_ERROR_NOT_PROCESSED							 = -1007,

	XCHG_ERROR_OBSOLETE_FUNCTION                         = -1008, 

	XCHG_ERROR_ATTRIBUTE_NOT_FOUND                        = -1009, 

	XCHG_WARNING_INVALID_ARGUMENT = -1010,

	XCHG_ERROR_INVALID_COMPONENT = -1011,

	XCHG_ERROR_INVALID_NODE = -1012,

	XCHG_ERROR_OUT_OF_TIME = -1013,

	XCHG_WARNING_DEFAULT_VALUE = -1014,

	XCHG_ERROR_MAX_FILE_NAME_SIZE = -1015,

};




typedef XchgErrorStatus Xchg_ErrorStatus;

// int xchgTypeError(int errNumber, wchar_t **outError);

#endif

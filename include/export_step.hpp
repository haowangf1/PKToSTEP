#pragma once
#include<string>
/* @brief Macro defines */

/* @brief Exports */
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#define AMXT_API EMSCRIPTEN_KEEPALIVE
#elif defined(_MSC_VER) && defined(BUILD_SHARED)
#define AMXT_API __declspec(dllexport)
#elif defined(__GNUG__) && defined(BUILD_SHARED)
#define AMXT_API __attribute__((visibility("default")))
#else
#define AMXT_API
#endif

/* @brief Error codes */

typedef int AMXT_STP_ERROR_code_t;

#define AMXT_STP_ERROR_OFFSET (0x30200000)
#define AMXT_STP_ERROR_no_errors ((AMXT_STP_ERROR_code_t)0)
#define AMXT_STP_ERROR_invalid_context ((AMXT_STP_ERROR_code_t)AMXT_STP_ERROR_OFFSET + 0x01)
#define AMXT_STP_ERROR_invalid_input ((AMXT_STP_ERROR_code_t)AMXT_STP_ERROR_OFFSET + 0x03)
#define AMXT_STP_ERROR_entity_not_found ((AMXT_STP_ERROR_code_t)AMXT_STP_ERROR_OFFSET + 0x04)
#define AMXT_STP_ERROR_unexpected_entity ((AMXT_STP_ERROR_code_t)AMXT_STP_ERROR_OFFSET + 0x05)
#define AMXT_STP_ERROR_entity_parameter_invalid ((AMXT_STP_ERROR_code_t)AMXT_STP_ERROR_OFFSET + 0x06)
#define AMXT_STP_ERROR_not_implemented ((AMXT_STP_ERROR_code_t)AMXT_STP_ERROR_OFFSET + 0x07)
#define AMXT_STP_ERROR_null_pointer ((AMXT_STP_ERROR_code_t)AMXT_STP_ERROR_OFFSET + 0x08)
#define AMXT_STP_ERROR_point_failure ((AMXT_STP_ERROR_code_t)AMXT_STP_ERROR_OFFSET + 0x09)
#define AMXT_STP_ERROR_curve_failure ((AMXT_STP_ERROR_code_t)AMXT_STP_ERROR_OFFSET + 0x0A)
#define AMXT_STP_ERROR_surface_failure ((AMXT_STP_ERROR_code_t)AMXT_STP_ERROR_OFFSET + 0x0B)
#define AMXT_STP_ERROR_vertex_failure ((AMXT_STP_ERROR_code_t)AMXT_STP_ERROR_OFFSET + 0x0C)
#define AMXT_STP_ERROR_edge_failure ((AMXT_STP_ERROR_code_t)AMXT_STP_ERROR_OFFSET + 0x0D)
#define AMXT_STP_ERROR_coedge_failure ((AMXT_STP_ERROR_code_t)AMXT_STP_ERROR_OFFSET + 0x0E)
#define AMXT_STP_ERROR_loop_failure ((AMXT_STP_ERROR_code_t)AMXT_STP_ERROR_OFFSET + 0x0F)
#define AMXT_STP_ERROR_face_failure ((AMXT_STP_ERROR_code_t)AMXT_STP_ERROR_OFFSET + 0x10)
#define AMXT_STP_ERROR_shell_failure ((AMXT_STP_ERROR_code_t)AMXT_STP_ERROR_OFFSET + 0x11)
#define AMXT_STP_ERROR_solid_failure ((AMXT_STP_ERROR_code_t)AMXT_STP_ERROR_OFFSET + 0x12)
#define AMXT_STP_ERROR_body_failure ((AMXT_STP_ERROR_code_t)AMXT_STP_ERROR_OFFSET + 0x13)
#define AMXT_STP_ERROR_assembly_failure ((AMXT_STP_ERROR_code_t)AMXT_STP_ERROR_OFFSET + 0x14)
#define AMXT_STP_ERROR_out_of_range ((AMXT_STP_ERROR_code_t)AMXT_STP_ERROR_OFFSET + 0x15)
#define AMXT_STP_ERROR_out_of_memory ((AMXT_STP_ERROR_code_t)AMXT_STP_ERROR_OFFSET + 0x16)
#define AMXT_STP_ERROR_internal ((AMXT_STP_ERROR_code_t)AMXT_STP_ERROR_OFFSET + 0x17)


/* @brief Structures */
/* AMXT_STP_export_o_t */
struct AMXT_STP_export_o_s
{
	int o_t_version; ///< version number
};

typedef struct AMXT_STP_export_o_s AMXT_STP_export_o_t;

/* AMXT_STP_export_o_m */
#define AMXT_STP_export_o_m(options) ((options).o_t_version = 1)

/* @brief Forward Declarations */
template<typename T>
class SmartPtr;
class Xchg_MainDoc;
typedef SmartPtr<Xchg_MainDoc> Xchg_MainDocPtr;

/**
 * @brief This function export  STEP file from a Xchg_MainDoc
 * @param[in] mainDoc 
 * @param[in] filename The options
 */
AMXT_API AMXT_STP_ERROR_code_t AMXT_STP_export(
	/******* returned arguments *******/
	Xchg_MainDocPtr* mainDoc,
    AMXT_STP_export_o_t* opt,
    std::string filename
);

// clang-format on

#ifndef XCHG_ERROR_CODE_HPP
#define XCHG_ERROR_CODE_HPP

typedef int STEPExport_ErrorCode;

constexpr STEPExport_ErrorCode STEP_OK                     = 0;
constexpr STEPExport_ErrorCode STEP_ERR_KERNEL             = 1;
constexpr STEPExport_ErrorCode STEP_ERR_NULL_ARGUMENT      = 2;
constexpr STEPExport_ErrorCode STEP_ERR_UNSUPPORTED_TYPE   = 3;
constexpr STEPExport_ErrorCode STEP_ERR_INTERNAL           = 4;

#endif // XCHG_ERROR_CODE_HPP

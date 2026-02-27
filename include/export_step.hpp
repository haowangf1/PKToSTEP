#pragma once
#include <string>

typedef int AMXT_STP_ERROR_code_t;

/* @brief Forward Declarations */
template<typename T>
class SmartPtr;
class Xchg_MainDoc;
typedef SmartPtr<Xchg_MainDoc> Xchg_MainDocPtr;

AMXT_STP_ERROR_code_t AMXT_STP_export(
    	/******* received arguments ******/
        const std::string& filename,
        Xchg_MainDocPtr maindoc
);
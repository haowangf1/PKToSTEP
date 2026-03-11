#include "../include/export_step.hpp"
#include "../include/xchg_to_step_writer.hpp"
#include "xchg_componentinstance.hpp"
#include "xchg_maindoc.hpp"

AMXT_API AMXT_STP_ERROR_code_t AMXT_STP_export(
    Xchg_MainDocPtr* mainDoc,
    AMXT_STP_export_o_t* opt,
    std::string filename
) {
    if (!mainDoc || !(*mainDoc)) {
        return AMXT_STP_ERROR_null_pointer;
    }

    try {
        XchgToSTEPWriter writer(filename);
        // Xchg 内部单位固定为米，无需设置 LengthUnit
        writer.SetPrecision(1e-6);

        bool success = writer.WriteMainDoc(*mainDoc);
        if (!success) {
            return AMXT_STP_ERROR_internal;
        }

        success = writer.Done();
        if (!success) {
            return AMXT_STP_ERROR_internal;
        }

        return AMXT_STP_ERROR_no_errors;
    } catch (...) {
        return AMXT_STP_ERROR_internal;
    }
}
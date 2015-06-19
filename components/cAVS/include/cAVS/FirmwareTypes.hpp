/*
 ********************************************************************************
 *                              INTEL CONFIDENTIAL
 *   Copyright(C) 2015 Intel Corporation. All Rights Reserved.
 *   The source code contained  or  described herein and all documents related to
 *   the source code ("Material") are owned by Intel Corporation or its suppliers
 *   or licensors.  Title to the  Material remains with  Intel Corporation or its
 *   suppliers and licensors. The Material contains trade secrets and proprietary
 *   and  confidential  information of  Intel or its suppliers and licensors. The
 *   Material  is  protected  by  worldwide  copyright  and trade secret laws and
 *   treaty  provisions. No part of the Material may be used, copied, reproduced,
 *   modified, published, uploaded, posted, transmitted, distributed or disclosed
 *   in any way without Intel's prior express written permission.
 *   No license  under any  patent, copyright, trade secret or other intellectual
 *   property right is granted to or conferred upon you by disclosure or delivery
 *   of the Materials,  either expressly, by implication, inducement, estoppel or
 *   otherwise.  Any  license  under  such  intellectual property  rights must be
 *   express and approved by Intel in writing.
 *
 ********************************************************************************
 */

#pragma once

#include <inttypes.h>

/** C_ASSERT is a windows specific macro for static assertion. Defining a linux equivalent in order
 *  to make the firmware header cross-toolchain. */
#ifdef __GNUC__
#define C_ASSERT(expr) static_assert(expr, "Static assertion failed")
#endif

namespace debug_agent
{
namespace cavs
{

/** This header already defines the dsp_fw namespace, so including it into debug_agent::cavs ...*/
#include "basefw_config.h"

/** Including the firmware types into the debug_agent::cavs::dsp_fw namespace */
namespace dsp_fw
{
#include "fw_manifest_common.h"
#include "adsp_ixc_status.h"

/** Defining this missing structure (not yet implemented in firmware headers) */
struct ModulesInfo
{
    uint32_t module_count;
    ModuleEntry module_info[1];
};
C_ASSERT(sizeof(ModulesInfo) == 120);

/** Max module count, used to retrieve module entries. */
static const uint32_t MaxModuleCount = 128;

/** Defining the missing MODULES_INFO_GET value of the enum BaseFwParams */
static const BaseFwParams MODULES_INFO_GET = static_cast<BaseFwParams>(9);
}


}
}

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

/** OED Driver structure uses the NTSTATUS type, which is almost a kernel type. In user mode some
 * NTSTATUS values are missing, for instance STATUS_SUCCESS. But we need this value to test driver
 * result, that's why the header ntstatus.h is included.
 *
 * But including both ntstatus.h (which is basically a driver header) and windows.h makes warnings
 * (macro redefinition)
 *
 * That's why we are using a macro trick to avoid these warnings
 * @see http://kirkshoop.blogspot.com/2011/09/ntstatus.html
 *
 * Defining the macro WIN32_NO_STATUS avoids to define twice NTSTATUS values.
 */
#define WIN32_NO_STATUS
#include <Windows.h>
#undef WIN32_NO_STATUS
#include <ntstatus.h>

namespace debug_agent
{
namespace cavs
{
namespace windows
{
namespace driver
{

/** Including the driver structures into the debug_agent::cavs::driver namespace */
#include <IIntcPrivateIOCTL.h>

/** Providing the missing ModuleParameterAccess structure */
#include <pshpack1.h>
struct ModuleParameterAccess
{
    uint32_t FwStatus; /* out, uint32_t */
    uint16_t ModuleId; /* in */
    uint16_t InstanceId; /* in */
    uint32_t ModuleParameterId; /* in */
    uint32_t ModuleParameterSize; /* in */
    char Parameter[1]; /* in/out */
};
#include <poppack.h>

/** Providing the missing value FEATURE_MODULE_PARAMETER_ACCESS of the enum IOCTL_FEATURE */
static const IOCTL_FEATURE FEATURE_MODULE_PARAMETER_ACCESS = static_cast<IOCTL_FEATURE>(0x240000);
}

}
}
}

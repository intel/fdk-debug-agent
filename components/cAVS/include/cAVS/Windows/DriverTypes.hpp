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

#include "cAVS/Windows/WindowsTypes.hpp"

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

/** Providing the missing driver structures (keeping driver coding style)*/

enum LOG_STATE
{
    STOPPED = 0,
    STARTED = 1
};

enum LOG_LEVEL
{
    CRITICAL = 2,
    HIGH = 3,
    MEDIUM = 4,
    LOW = 5,
    VERBOSE = 6
};

enum LOG_OUTPUT
{
    OUTPUT_SRAM = 0,
    OUTPUT_PTI = 1
};

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

struct FwLogsState
{
    uint32_t started;
    uint32_t level;
    uint32_t output;
};

#include <poppack.h>

/** Providing the missing values of the enum IOCTL_FEATURE */
static const IOCTL_FEATURE FEATURE_LOG_PARAMETERS = static_cast<IOCTL_FEATURE>(0x200000);
static const IOCTL_FEATURE FEATURE_MODULE_PARAMETER_ACCESS = static_cast<IOCTL_FEATURE>(0x240000);
}

}
}
}

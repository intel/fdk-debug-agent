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

#include "cAVS/DspFw/Common.hpp"
#include "Util/WrappedRaw.hpp"

namespace debug_agent
{
namespace cavs
{
namespace dsp_fw
{

/* Importing IxcStatus enum */
using IxcStatus = private_fw::Message::IxcStatus;

/* Importing BaseFwParams enum */
using BaseFwParams = private_fw::dsp_fw::BaseFwParams;

/* Importing BaseFwParams enum */
using BaseModuleParams = private_fw::dsp_fw::BaseModuleParams;

static inline ParameterId toParameterId(BaseFwParams param)
{
    return ParameterId{static_cast<ParameterId::RawType>(param)};
}

namespace detail
{
struct ExtendedIdTrait
{
    using RawType = uint32_t;
};
} // namespace detail

/** Represent the id of a base fw sub component.
 * The base firmware has several module like components in it.
 * To address them, the ParameterId is splited in a type and an instance part.
 */
using BaseFwInstanceId = util::WrappedRaw<detail::ExtendedIdTrait>;

static inline ParameterId toParameterId(BaseModuleParams param)
{
    return ParameterId{static_cast<ParameterId::RawType>(param)};
}
}
}
}

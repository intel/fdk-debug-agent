/*
********************************************************************************
*                              INTEL CONFIDENTIAL
*   Copyright(C) 2016 Intel Corporation. All Rights Reserved.
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

#include "Util/EnumHelper.hpp"

namespace debug_agent
{
namespace cavs
{

class Perf
{
public:
    enum class State : uint32_t
    {
        Disabled = 0,
        Stopped = 1,
        Started = 2,
        Paused = 3
    };
    static const util::EnumHelper<State> &stateHelper()
    {
        static const util::EnumHelper<State> helper({{State::Started, "Started"},
                                                     {State::Paused, "Paused"},
                                                     {State::Stopped, "Stopped"},
                                                     {State::Disabled, "Disabled"}});
        return helper;
    }

    enum class PowerMode : uint32_t
    {
        D0 = 0,
        D0i3 = 1
    };
    static const util::EnumHelper<PowerMode> &powerModeHelper()
    {
        static const util::EnumHelper<PowerMode> helper(
            {{PowerMode::D0, "D0"}, {PowerMode::D0i3, "D0i3"}});
        return helper;
    }

    struct Item
    {
        uint32_t resourceId;
        PowerMode powerMode;
        uint32_t budget;
        uint32_t peak;
        uint32_t average;
    };
};
}
}

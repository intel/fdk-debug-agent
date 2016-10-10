/*
 * Copyright (c) 2016, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#pragma once

#include "Util/EnumHelper.hpp"
#include "Util/Exception.hpp"
#include <string>

namespace debug_agent
{
namespace cavs
{

class Perf
{
public:
    using Exception = util::Exception<Perf>;
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
        std::string uuid;
        uint16_t instanceId;
        PowerMode powerMode;
        bool isRemoved;
        uint32_t budget;
        uint32_t peak;
        uint32_t average;
    };

    virtual ~Perf() = default;

    virtual State getState() = 0;
    virtual void setState(State) = 0;
};
}
}

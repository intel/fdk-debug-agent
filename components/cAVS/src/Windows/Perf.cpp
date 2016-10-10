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
#include "cAVS/Windows/Perf.hpp"
#include "cAVS/Windows/IoCtlDescription.hpp"
#include "cAVS/Windows/IoctlHelpers.hpp"

namespace debug_agent
{
namespace cavs
{
namespace windows
{
using ioctl_helpers::ioctl;
using driver::IoCtlType;
using GetState =
    IoCtlDescription<IoCtlType::TinyGet, driver::IOCTL_FEATURE::FEATURE_PERFORMANCE_MEASUREMENT,
                     driver::PERFORMANCE_MEASUREMENT::PERFORMANCE_MEASURE_PARAM, uint32_t>;
using SetState =
    IoCtlDescription<IoCtlType::TinySet, driver::IOCTL_FEATURE::FEATURE_PERFORMANCE_MEASUREMENT,
                     driver::PERFORMANCE_MEASUREMENT::PERFORMANCE_MEASURE_PARAM, uint32_t>;

void Perf::setState(Perf::State state)
{
    auto hack = static_cast<SetState::Data>(state);
    ioctl<SetState, Perf::Exception>(mDevice, hack);
}

Perf::State Perf::getState()
{
    GetState::Data state{0xffffffff};
    ioctl<GetState, Perf::Exception>(mDevice, state);
    return static_cast<Perf::State>(state);
}
}
}
}

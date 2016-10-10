/*
 * Copyright (c) 2015, Intel Corporation
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
#include <cAVS/DspFw/Probe.hpp>
#include "TestCommon/TestHelpers.hpp"
#include <catch.hpp>

using namespace debug_agent::cavs::dsp_fw;

TEST_CASE("ProbePointId validity")
{
    using namespace std;

    uint32_t illegalModuleId = 1 << probe_point_id::moduleIdSize;
    uint32_t illegalInstanceId = 1 << probe_point_id::instanceIdSize;
    ProbeType illegalType = static_cast<ProbeType>(1 << probe_point_id::typeSize);
    uint32_t illegalIndex = 1 << probe_point_id::indexSize;

    CHECK_THROWS_AS_MSG(ProbePointId(illegalModuleId, 0, ProbeType::Input, 0),
                        ProbePointId::Exception,
                        "Module id too large (" + to_string(illegalModuleId) + ")");
    CHECK_THROWS_AS_MSG(ProbePointId(0, illegalInstanceId, ProbeType::Input, 0),
                        ProbePointId::Exception,
                        "Instance id too large (" + to_string(illegalInstanceId) + ")");
    CHECK_THROWS_AS_MSG(ProbePointId(0, 0, illegalType, 0), ProbePointId::Exception,
                        "Invalid probe type (" + to_string(static_cast<uint32_t>(illegalType)) +
                            ")");
    CHECK_THROWS_AS_MSG(ProbePointId(0, 0, ProbeType::Input, illegalIndex), ProbePointId::Exception,
                        "Pin index too large (" + to_string(illegalIndex) + ")");
}

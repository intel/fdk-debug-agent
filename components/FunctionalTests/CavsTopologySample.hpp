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

#pragma once

#include "cAVS/DspFw/Gateway.hpp"
#include "cAVS/DspFw/Pipeline.hpp"
#include "cAVS/DspFw/ModuleInstance.hpp"
#include "cAVS/DspFw/ModuleType.hpp"
#include "cAVS/DspFw/Scheduler.hpp"
#include "Util/Buffer.hpp"

namespace debug_agent
{

/* This class creates firmware objects that match the topology sample available in the SwAS. */
class CavsTopologySample final
{
public:
    static void createInstanceFirmwareObjects(
        std::vector<cavs::dsp_fw::ModuleInstanceProps> &moduleInstances,
        std::vector<cavs::dsp_fw::GatewayProps> &gateways,
        std::vector<cavs::dsp_fw::PipeLineIdType> &pipelineIds,
        std::vector<cavs::dsp_fw::PplProps> &pipelines,
        std::vector<cavs::dsp_fw::SchedulersInfo> &schedulers);

    static void createFirmwareObjects(std::vector<cavs::dsp_fw::ModuleEntry> &modules,
                                      util::Buffer &fwConfig, util::Buffer &hwConfig);

    static const size_t maxPplCount;
    static const size_t gatewaysCount;
    static constexpr size_t dspCoreCount = 1;
    static constexpr size_t maxModInstCount = 4;

private:
    CavsTopologySample();
};
}

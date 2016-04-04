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

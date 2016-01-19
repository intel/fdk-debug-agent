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
#include "cAVS/DspFw/ModuleType.hpp"
#include "cAVS/DspFw/ModuleInstance.hpp"
#include "cAVS/DspFw/FwConfig.hpp"
#include "cAVS/DspFw/HwConfig.hpp"
#include "cAVS/DspFw/Pipeline.hpp"
#include "cAVS/DspFw/Gateway.hpp"
#include "cAVS/DspFw/Scheduler.hpp"
#include <catch.hpp>

using namespace debug_agent::util;
using namespace debug_agent::cavs::dsp_fw;

static const AudioDataFormatIpc audioFormat = {static_cast<SamplingFrequency>(1),
                                               static_cast<BitDepth>(2),
                                               static_cast<ChannelMap>(3),
                                               static_cast<ChannelConfig>(4),
                                               static_cast<InterleavingStyle>(5),
                                               6,
                                               7,
                                               static_cast<SampleType>(8),
                                               9};

#define AUDIOFORMAT_MEMORY 1, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 4, 0, 0, 0, 5, 0, 0, 0, 6, 7, 8, 9

#define HASH_MEMORY                                                                                \
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,    \
        12, 13, 14, 15

/* This template method tests serialization, deserialization and operator == of the supplied
 *  type.
 */
template <typename T>
void testType(const T &expectedValue, const Buffer &expectedBuffer)
{
    ByteStreamWriter writer;
    writer.write(expectedValue);
    CHECK(writer.getBuffer() == expectedBuffer);

    ByteStreamReader reader(expectedBuffer);
    T readValue;
    reader.read(readValue);
    CHECK(readValue == expectedValue);
}

/** AUDIOFORMAT */

TEST_CASE("FirmwareTypes : AudioDataFormatIpc")
{
    testType(audioFormat, {AUDIOFORMAT_MEMORY});
}

/** PIPELINE */

TEST_CASE("FirmwareTypes : PipelinesListInfo")
{
    testType(PipelinesListInfo{{1, 3}}, {2, 0, 0, 0, 1, 0, 0, 0, 3, 0, 0, 0});
}

TEST_CASE("FirmwareTypes : PplProps")
{
    testType(PplProps{1, 2, 3, 4, 5, 6, {{1, 6}, {2, 7}, {3, 8}}, {4, 5}, {}},
             {
                 1, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 4, 0, 0, 0, 5, 0, 0, 0, 6, 0, 0, 0,

                 3, 0, 0, 0, 6, 0, 1, 0, 7, 0, 2, 0, 8, 0, 3, 0,

                 2, 0, 0, 0, 4, 0, 0, 0, 5, 0, 0, 0,

                 0, 0, 0, 0,
             });
}

/** SCHEDULER */

TEST_CASE("FirmwareTypes : TaskProps")
{
    testType(TaskProps{1, {{2, 3}, {4, 5}}}, {1, 0, 0, 0, 2, 0, 0, 0, 3, 0, 2, 0, 5, 0, 4, 0});
}

TEST_CASE("FirmwareTypes : SchedulderProps")
{
    TaskProps task1 = {3, {{1, 3}, {2, 4}}};
    TaskProps task2 = {4, {{8, 5}}};

    SchedulerProps props = {1, 2, {task1, task2}};

    testType(props, {
                        1, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0,

                        3, 0, 0, 0, 2, 0, 0, 0, 3, 0, 1, 0, 4, 0, 2, 0,

                        4, 0, 0, 0, 1, 0, 0, 0, 5, 0, 8, 0,
                    });
}

TEST_CASE("FirmwareTypes : SchedulersInfo")
{
    TaskProps task1 = {3, {{1, 0}, {2, 0}}};
    TaskProps task2 = {4, {{8, 0}}};
    TaskProps task3 = {6, {}};

    SchedulerProps props1 = {1, 2, {task1, task2}};
    SchedulerProps props2 = {4, 2, {task3}};

    SchedulersInfo infos = {{props1, props2}};

    testType(infos, {
                        2, 0, 0, 0,

                        1, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0,

                        3, 0, 0, 0, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 0,

                        4, 0, 0, 0, 1, 0, 0, 0, 0, 0, 8, 0,

                        4, 0, 0, 0, 2, 0, 0, 0, 1, 0, 0, 0,

                        6, 0, 0, 0, 0, 0, 0, 0,
                    });
}

/** MODULE INSTANCES */

TEST_CASE("FirmwareTypes : PinProps")
{
    testType(PinProps{StreamType::STREAM_TYPE_PCM, audioFormat, 2},
             {
                 static_cast<uint8_t>(StreamType::STREAM_TYPE_PCM), 0, 0, 0, AUDIOFORMAT_MEMORY, 2,
                 0, 0, 0,
             });
}

TEST_CASE("FirmwareTypes : PinListInfo")
{
    testType(PinListInfo{{{StreamType::STREAM_TYPE_PCM, audioFormat, 3}}},
             {1, 0, 0, 0, static_cast<uint8_t>(StreamType::STREAM_TYPE_PCM), 0, 0, 0,
              AUDIOFORMAT_MEMORY, 3, 0, 0, 0});
}

TEST_CASE("FirmwareTypes : ModuleInstanceProps")
{
    static const PinListInfo input_pins = {{{static_cast<StreamType>(1), audioFormat, 3}}};
    static const PinListInfo output_pins = {{{static_cast<StreamType>(4), audioFormat, 5},
                                             {static_cast<StreamType>(6), audioFormat, 7}}};

    static const ModuleInstanceProps instanceProps = {{1, 9},
                                                      2,
                                                      3,
                                                      4,
                                                      5,
                                                      6,
                                                      7,
                                                      8,
                                                      9,
                                                      10,
                                                      11,
                                                      input_pins,
                                                      output_pins,
                                                      ConnectorNodeId(12),
                                                      ConnectorNodeId(13)};

    testType(instanceProps, {9, 0, 1, 0, 2, 0, 0, 0, 3, 0, 0, 0, 4, 0, 0, 0, 5, 0, 0, 0, 6, 0, 0, 0,
                             7, 0, 0, 0, 8, 0, 0, 0, 9, 0, 0, 0, 10, 0, 0, 0, 11, 0, 0, 0,

                             1, 0, 0, 0, 1, 0, 0, 0, AUDIOFORMAT_MEMORY, 3, 0, 0, 0,

                             2, 0, 0, 0, 4, 0, 0, 0, AUDIOFORMAT_MEMORY, 5, 0, 0, 0, 6, 0, 0, 0,
                             AUDIOFORMAT_MEMORY, 7, 0, 0, 0,

                             12, 0, 0, 0, 13, 0, 0, 0});
}

/** GATEWAY */

TEST_CASE("FirmwareTypes : GatewayProps")
{
    testType(GatewayProps{1, 2}, {1, 0, 0, 0, 2, 0, 0, 0});
}

TEST_CASE("FirmwareTypes : GatewaysInfo")
{
    testType(GatewaysInfo{{{1, 2}, {3, 4}}},
             {
                 2, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 4, 0, 0, 0,
             });
}

/** MODULE TYPE */

TEST_CASE("FirmwareTypes : SegmentFlags")
{
    testType(SegmentFlags{3},
             {
                 3, 0, 0, 0,
             });
}

TEST_CASE("FirmwareTypes : SegmentDesc")
{
    testType(SegmentDesc{{1}, 2, 3},
             {
                 1, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0,
             });
}

TEST_CASE("FirmwareTypes : ModuleType")
{
    testType(ModuleType{3},
             {
                 3, 0, 0, 0,
             });
}

TEST_CASE("FirmwareTypes : ModuleEntry and ModulesInfo")
{
    const ModuleEntry module{1,
                             0,
                             {0, 1, 2, 3, 4, 5, 6, 7},
                             {1, 2, 3, 4},
                             {5},
                             {HASH_MEMORY},
                             1,
                             2,
                             3,
                             4,
                             5,
                             6,
                             {{{1}, 2, 3}, {{4}, 5, 6}, {{7}, 8, 9}}};

    const Buffer moduleMemory{1,           0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7,

                              1,           0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 4, 0, 0, 0,

                              5,           0, 0, 0,

                              HASH_MEMORY,

                              1,           0, 0, 0, 2, 0, 3, 0, 4, 0, 0, 0, 5, 0, 6, 0,

                              1,           0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 4, 0, 0, 0, 5, 0,
                              0,           0, 6, 0, 0, 0, 7, 0, 0, 0, 8, 0, 0, 0, 9, 0, 0, 0};

    testType(module, moduleMemory);

    ModulesInfo modulesInfo;
    modulesInfo.module_info.push_back(module);

    /* Putting array size */
    Buffer modulesInfoMemory = {1, 0, 0, 0};

    /* Putting module entry content */
    modulesInfoMemory.insert(modulesInfoMemory.end(), moduleMemory.begin(), moduleMemory.end());

    testType(modulesInfo, modulesInfoMemory);
}

/** CONFIG */

TEST_CASE("FirmwareTypes : FwVersion")
{
    testType(FwVersion{1, 2, 3, 4}, {1, 0, 2, 0, 3, 0, 4, 0});
}

TEST_CASE("FirmwareTypes : DmaBufferConfig")
{
    testType(DmaBufferConfig{1, 2}, {1, 0, 0, 0, 2, 0, 0, 0});
}

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
#include "cAVS/DynamicSizedFirmwareTypes.hpp"
#include "catch.hpp"

using namespace debug_agent::util;
using namespace debug_agent::cavs::dsp_fw;

static const AudioDataFormatIpc audioFormat = {
    static_cast<SamplingFrequency>(1),
    static_cast<BitDepth>(2),
    static_cast<ChannelMap>(3),
    static_cast<ChannelConfig>(4),
    static_cast<InterleavingStyle>(5),
    6,
    7,
    static_cast<SampleType>(8),
    9
};

#define AUDIOFORMAT_MEMORY \
    1, 0, 0, 0, \
    2, 0, 0, 0, \
    3, 0, 0, 0, \
    4, 0, 0, 0, \
    5, 0, 0, 0, \
    6, 7, 8, 9

TEST_CASE("DSFirmwareTypes : DSPplProps")
{
    DSPplProps props = { 1, 2, 3, 4, 5, 6, { { 1, 6 }, { 2, 7 }, { 3, 8 } }, { 4, 5 }, {} };

    const std::vector<uint8_t> expected = {
        1, 0, 0, 0,
        2, 0, 0, 0,
        3, 0, 0, 0,
        4, 0, 0, 0,
        5, 0, 0, 0,
        6, 0, 0, 0,

        3, 0, 0, 0,
        6, 0, 1, 0,
        7, 0, 2, 0,
        8, 0, 3, 0,

        2, 0, 0, 0,
        4, 0, 0, 0,
        5, 0, 0, 0,

        0, 0, 0, 0,
    };

    ByteStreamWriter writer;
    props.toStream(writer);

    CHECK(writer.getBuffer() == expected);

    ByteStreamReader reader(expected);
    DSPplProps readProps;
    readProps.fromStream(reader);

    CHECK(readProps == props);
}

TEST_CASE("DSFirmwareTypes : DSSchedulersInfo")
{
    DSTaskProps task1 = { 3, { { 1, 0 }, { 2, 0} } };
    DSTaskProps task2 = { 4, { { 8, 0 } } };
    DSTaskProps task3 = { 6, {} };

    DSSchedulerProps props1 = { 1, 2, { task1, task2 } };
    DSSchedulerProps props2 = { 4, 2, { task3 } };

    DSSchedulersInfo infos = { { props1, props2 } };

    const std::vector<uint8_t> expected = {
        2, 0, 0, 0,

        1, 0, 0, 0,
        2, 0, 0, 0,
        2, 0, 0, 0,

        3, 0, 0, 0,
        2, 0, 0, 0,
        0, 0, 1, 0,
        0, 0, 2, 0,

        4, 0, 0, 0,
        1, 0, 0, 0,
        0, 0, 8, 0,

        4, 0, 0, 0,
        2, 0, 0, 0,
        1, 0, 0, 0,

        6, 0, 0, 0,
        0, 0, 0, 0,
    };

    ByteStreamWriter writer;
    infos.toStream(writer);

    CHECK(writer.getBuffer() == expected);

    ByteStreamReader reader(expected);
    DSSchedulersInfo readInfos;
    readInfos.fromStream(reader);

    CHECK(readInfos == infos);
}

TEST_CASE("DSFirmwareTypes : DSPinListInfo")
{
    static const DSPinListInfo list = {{
        { static_cast<StreamType>(1), audioFormat, 3 }
    }};

    const std::vector<uint8_t> expected = {
        1, 0, 0, 0,
        1, 0, 0, 0,
        AUDIOFORMAT_MEMORY,
        3, 0, 0, 0
    };

    ByteStreamWriter writer;
    list.toStream(writer);

    CHECK(writer.getBuffer() == expected);

    ByteStreamReader reader(expected);
    DSPinListInfo readList;
    readList.fromStream(reader);

    CHECK(readList == list);
}

TEST_CASE("DSFirmwareTypes : DSModuleInstanceProps")
{
    static const DSPinListInfo input_pins = { {
        { static_cast<StreamType>(1), audioFormat, 3 }
    } };
    static const DSPinListInfo output_pins = { {
        { static_cast<StreamType>(4), audioFormat, 5 },
        { static_cast<StreamType>(6), audioFormat, 7 }
    } };

    static const DSModuleInstanceProps instanceProps = {
        { 1, 9 }, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, input_pins, output_pins,
        ConnectorNodeId(12), ConnectorNodeId(13)
    };

    static const std::vector<uint8_t> expected = {
        9, 0, 1, 0,
        2, 0, 0, 0,
        3, 0, 0, 0,
        4, 0, 0, 0,
        5, 0, 0, 0,
        6, 0, 0, 0,
        7, 0, 0, 0,
        8, 0, 0, 0,
        9, 0, 0, 0,
        10, 0, 0, 0,
        11, 0, 0, 0,

        1, 0, 0, 0,
        1, 0, 0, 0,
        AUDIOFORMAT_MEMORY,
        3, 0, 0, 0,

        2, 0, 0, 0,
        4, 0, 0, 0,
        AUDIOFORMAT_MEMORY,
        5, 0, 0, 0,
        6, 0, 0, 0,
        AUDIOFORMAT_MEMORY,
        7, 0, 0, 0,

        12, 0, 0, 0,
        13, 0, 0, 0
    };

    ByteStreamWriter writer;
    instanceProps.toStream(writer);

    CHECK(writer.getBuffer() == expected);

    ByteStreamReader reader(expected);
    DSModuleInstanceProps readInstanceProps;
    readInstanceProps.fromStream(reader);

    CHECK(readInstanceProps == instanceProps);
}


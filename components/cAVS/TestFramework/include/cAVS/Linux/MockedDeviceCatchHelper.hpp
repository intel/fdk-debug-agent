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

#include "cAVS/Linux/MockedDevice.hpp"
#include "cAVS/Linux/MockedControlDevice.hpp"
#include "cAVS/Linux/MockedCompressDevice.hpp"
#include <catch.hpp>

namespace debug_agent
{
namespace cavs
{
namespace linux
{

struct MockedDeviceFixture
{
    std::unique_ptr<MockedDevice> device = std::make_unique<MockedDevice>([] {
        INFO("There are leftover test inputs");
        CHECK(false);
    });

    std::unique_ptr<MockedControlDevice> controlDevice =
        std::make_unique<MockedControlDevice>("myMockedControlCard", [] {
            INFO("There are leftover test inputs");
            CHECK(false);
        });

    std::unique_ptr<MockedCompressDevice> compressDevice =
        std::make_unique<MockedCompressDevice>(compress::DeviceInfo{0, 5},
                                               [] {
                                                   INFO("There are leftover test inputs");
                                                   CHECK(false);
                                               });
    std::unique_ptr<MockedCompressDevice> compressProbeInjectDevice =
        std::make_unique<MockedCompressDevice>(compress::DeviceInfo{0, 6},
                                               [] {
                                                   INFO("There are leftover test inputs");
                                                   CHECK(false);
                                               });
};
}
}
}

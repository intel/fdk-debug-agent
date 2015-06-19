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

#include "cAVS/Windows/DeviceIdFinder.hpp"
#include <catch.hpp>
#include <Ntddstor.h>
#include <iostream>

using namespace debug_agent::cavs::windows;

const GUID InvalidGUID =
{ 0x00000000, 0x0000, 0x0000, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };

TEST_CASE("DeviceIdFinder: wrong guid")
{
    std::set<std::string> deviceIds;

    /* The guid is 0, therefore no device should be found */
    CHECK_THROWS_AS(DeviceIdFinder::findAll(InvalidGUID, deviceIds), DeviceIdFinder::Exception);
    CHECK_THROWS_AS(DeviceIdFinder::findOne(InvalidGUID), DeviceIdFinder::Exception);
}

TEST_CASE("DeviceIdFinder: Valid guid")
{
    std::set<std::string> deviceIds;

    /* using a the guid class GUID_DEVINTERFACE_VOLUME, which contains storage devices.
     * We assume that all machines have at least one storage device */
    CHECK_NOTHROW(DeviceIdFinder::findAll(GUID_DEVINTERFACE_VOLUME, deviceIds));
    CHECK_FALSE(deviceIds.empty());

    /* Printing found devices */
    std::cout << "Found storage devices: " << deviceIds.size() << std::endl;
    for (auto &deviceId : deviceIds) {
        std::cout << "  " << deviceId << std::endl;
    }

    /* Testing findOne with the first device as substring*/
    std::string oneDeviceId = *deviceIds.begin();
    CHECK_NOTHROW(DeviceIdFinder::findOne(GUID_DEVINTERFACE_VOLUME, oneDeviceId));

    if (deviceIds.size() > 1) {
        /* checking that findOne throws an exception if more than one device is returned*/
        CHECK_THROWS_AS(DeviceIdFinder::findOne(GUID_DEVINTERFACE_VOLUME),
            DeviceIdFinder::Exception);
    }
}

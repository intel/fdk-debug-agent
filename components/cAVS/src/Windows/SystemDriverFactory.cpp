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
#include "cAVS/SystemDriverFactory.hpp"
#include "cAVS/Windows/Driver.hpp"
#include "cAVS/Windows/SystemDevice.hpp"
#include "cAVS/Windows/DeviceIdFinder.hpp"
#include "cAVS/Windows/RealTimeWppClientFactory.hpp"
#include "cAVS/Windows/StubbedWppClientFactory.hpp"

namespace debug_agent
{
namespace cavs
{

/** OED driver interface substring */
static const std::string DriverInterfaceSubstr = "intelapp2audiodspiface";

/** OED driver class */
const GUID DriverInterfaceGuid =
{ 0xd562b888, 0xcf36, 0x4c54, { 0x84, 0x1d, 0x10, 0xff, 0x7b, 0xff, 0x4f, 0x60 } };

std::unique_ptr<Driver> cavs::SystemDriverFactory::newDriver() const
{
    /* Finding device id */
    std::string deviceId;
    try
    {
        deviceId = windows::DeviceIdFinder::findOne(DriverInterfaceGuid,
            DriverInterfaceSubstr);
    }
    catch (windows::DeviceIdFinder::Exception &e)
    {
        throw Exception("Cannot get device identifier: " + std::string(e.what()));
    }

    std::unique_ptr<windows::Device> device;
    try
    {
        device = std::make_unique<windows::SystemDevice>(deviceId);
    }
    catch (windows::Device::Exception &e)
    {
        throw Exception("Cannot create device: " + std::string(e.what()));
    }

    /* Creating the WppClientFactory */
    std::unique_ptr<windows::WppClientFactory> wppClientFactory;
    if (mLogControlOnly){
        wppClientFactory = std::make_unique<windows::StubbedWppClientFactory>();
    }
    else {
        wppClientFactory = std::make_unique<windows::RealTimeWppClientFactory>();
    }

    /* Creating Driver interface */
    return std::make_unique<windows::Driver>(std::move(device), std::move(wppClientFactory));
}

}
}

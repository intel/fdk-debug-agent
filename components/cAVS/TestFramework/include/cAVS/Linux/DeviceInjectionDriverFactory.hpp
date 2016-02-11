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

#include "cAVS/Linux/Device.hpp"
#include "cAVS/DriverFactory.hpp"
#include <memory>

namespace debug_agent
{
namespace cavs
{
namespace linux
{

/** This driver factory injects a device and a wpp client factory into the created
 * driver instance */
class DeviceInjectionDriverFactory : public DriverFactory
{
public:
    DeviceInjectionDriverFactory(std::unique_ptr<Device> injectedDevice)
        : mInjectedDevice(std::move(injectedDevice))
    {
    }

    std::unique_ptr<cavs::Driver> newDriver() const override;

private:
    /** newDriver() is a const method because the factory doesn't suppose to be changed when
     * an instance is created.
     * But in the DeviceInjectionDriverFactory case, the following unique pointer will loose its
     * content when newDriver() is called, so declaring it mutable */
    mutable std::unique_ptr<Device> mInjectedDevice;
};
}
}
}

/*
********************************************************************************
*                              INTEL CONFIDENTIAL
*   Copyright(C) 2015-2016 Intel Corporation. All Rights Reserved.
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
#include <cAVS/SystemDriverFactory.hpp>
#include <cAVS/Linux/Driver.hpp>
#include <cAVS/Linux/SystemDevice.hpp>
#include <cAVS/Linux/ControlDeviceFactory.hpp>
#include "cAVS/Linux/TinyCompressDeviceFactory.hpp"

namespace debug_agent
{
namespace cavs
{
static const std::string controlDevice{"control"};

std::unique_ptr<Driver> SystemDriverFactory::newDriver() const
{
    /* Creating the CompressDeviceFactory */
    std::unique_ptr<linux::CompressDeviceFactory> compressDeviceFactory(
        std::make_unique<linux::TinyCompressDeviceFactory>());

    assert(compressDeviceFactory != nullptr);

    /* Finding ALSA Device Card for control using compress Factory */
    const std::string controlCard{linux::AudioProcfsHelper::getDeviceType(controlDevice)};
    assert(!controlCard.empty());

    /* Creating the Control Device for the control Card */
    std::unique_ptr<linux::ControlDevice> controlDevice;
    try {
        linux::ControlDeviceFactory controlDeviceFactory;
        controlDevice = controlDeviceFactory.newControlDevice(controlCard);
    } catch (linux::ControlDevice::Exception &e) {
        throw Exception("Cannot create control device: " + std::string(e.what()));
    }
    assert(controlDevice != nullptr);

    std::unique_ptr<linux::Device> device;
    try {
        device = std::make_unique<linux::SystemDevice>();
    } catch (linux::Device::Exception &e) {
        throw Exception("Cannot create device: " + std::string(e.what()));
    }
    return std::make_unique<linux::Driver>(std::move(device), std::move(controlDevice),
                                           std::move(compressDeviceFactory));
}
}
}

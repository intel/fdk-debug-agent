/*
 * Copyright (c) 2015-2016, Intel Corporation
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
#include <cAVS/SystemDriverFactory.hpp>
#include <cAVS/Linux/Driver.hpp>
#include <cAVS/Linux/SystemDevice.hpp>
#include <cAVS/Linux/DebugFsEntryHandler.hpp>
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
        device =
            std::make_unique<linux::SystemDevice>(std::make_unique<linux::DebugFsEntryHandler>());
    } catch (linux::Device::Exception &e) {
        throw Exception("Cannot create device: " + std::string(e.what()));
    }
    return std::make_unique<linux::Driver>(std::move(device), std::move(controlDevice),
                                           std::move(compressDeviceFactory));
}
}
}

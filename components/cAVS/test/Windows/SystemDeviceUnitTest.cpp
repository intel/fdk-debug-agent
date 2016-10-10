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

#include "cAVS/Windows/SystemDevice.hpp"
#include "cAVS/Windows/DeviceIdFinder.hpp"
#include "Util/TypedBuffer.hpp"
#include <catch.hpp>
#include <memory>
#include <iostream>

/* Ntddser.h header is required to include ioctl structure and device GUID.
 * But for an unknown reason including Ntddser.h leads to macro redefinition warnings.
 * Some investigation has been performed, but without result. So disabling the warning,
 * it's acceptable for a functional test */
#pragma warning(push)
#pragma warning(disable : 4005)
#include <Ntddser.h>
#pragma warning(pop)

using namespace debug_agent::cavs::windows;
using namespace debug_agent::util;

/**
 * This functional test retrieves the baud rate of serial devices.
 * We assume that all machines have at least on serial device.
 */
TEST_CASE("SystemDevice: getting device baud rate")
{
    std::set<std::string> deviceIds;

    /* Finding devices matching guid GUID_DEVINTERFACE_COMPORT */
    CHECK_NOTHROW(DeviceIdFinder::findAll(GUID_DEVINTERFACE_COMPORT, deviceIds));
    CHECK_FALSE(deviceIds.empty());

    std::cout << "Serial devices found: " << deviceIds.size() << std::endl;

    /* Iterating on each device */
    for (auto &device : deviceIds) {
        try {
            /* Creating a system device using the current device identifier */
            SystemDevice systemDevice(device);

            /* Performing the io control. Input buffer is not used. */
            TypedBuffer<SERIAL_BAUD_RATE> output;
            systemDevice.ioControl(IOCTL_SERIAL_GET_BAUD_RATE, nullptr, &output);

            /* Printing the baud rate */
            std::cout << "  Baud rate: " << output->BaudRate << std::endl;
        } catch (Device::Exception &) {
            /* The Device::Exception is thrown by the SystemDevice constructor. The constructor
             * call cannot be surrounded by CHECK_NOTHROW because this macro uses a try/catch
             * block, making unreachable the created instance.
             * So using a try/catch block that surrounds the "device" local variable life,
             * and then re-throwing the exception within the CHECK_NOTHROW macro.
             */
            CHECK_NOTHROW(throw);
        }
    }
}

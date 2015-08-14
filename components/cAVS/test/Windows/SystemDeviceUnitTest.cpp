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

#include "cAVS/Windows/SystemDevice.hpp"
#include "cAVS/Windows/DeviceIdFinder.hpp"
#include <catch.hpp>
#include <memory>
#include <iostream>

/* Ntddser.h header is required to include ioctl structure and device GUID.
 * But for an unknown reason including Ntddser.h leads to macro redefinition warnings.
 * Some investigation has been performed, but without result. So disabling the warning,
 * it's acceptable for a functional test */
#pragma warning( push )
#pragma warning(disable:4005)
#include <Ntddser.h>
#pragma warning( pop )

using namespace debug_agent::cavs::windows;

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
    for (auto &device : deviceIds)
    {
        try {
            /* Creating a system device using the current device identifier */
            SystemDevice systemDevice(device);

            /* Performing the io control. Input buffer is not used. */
            TypedBuffer<SERIAL_BAUD_RATE> output;
            systemDevice.ioControl(IOCTL_SERIAL_GET_BAUD_RATE, nullptr, &output);

            /* Printing the baud rate */
            std::cout << "  Baud rate: " << output->BaudRate << std::endl;
        }
        catch (Device::Exception &)
        {
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


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

#include "cAVS/Linux/CompressDevice.hpp"
#include "cAVS/Linux/CompressTypes.hpp"
#include <memory>
#include <stdexcept>

namespace debug_agent
{
namespace cavs
{
namespace linux
{

class CompressDeviceFactory
{
public:
    struct Exception : std::logic_error
    {
        using std::logic_error::logic_error;
    };

    CompressDeviceFactory() = default;
    virtual ~CompressDeviceFactory() = default;

    /** @throw CompressDeviceFactory::Exception */
    virtual std::unique_ptr<CompressDevice> newCompressDevice(
        const compress::DeviceInfo &info) const = 0;

    /** get a list of logger device information found on the platform.
     * @return loggers info list.
     */
    virtual const compress::LoggersInfo getLoggerDeviceInfoList() const = 0;

    /** @todo provides methods to get probe devices. */

private:
    /* Make this class non copyable */
    CompressDeviceFactory(const CompressDeviceFactory &) = delete;
    CompressDeviceFactory &operator=(const CompressDeviceFactory &) = delete;
};
}
}
}

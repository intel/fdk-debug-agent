/*
 * Copyright (c) 2016, Intel Corporation
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

    /** get a list of injection probe device information found on the platform.
     * @return injection probe info list.
     */
    virtual const compress::InjectionProbesInfo getInjectionProbeDeviceInfoList() const = 0;

    /** get the extraction probe device information found on the platform.
     * @return extraction probe info.
     */
    virtual const compress::ExtractionProbeInfo getExtractionProbeDeviceInfo() const = 0;

    /** @todo provides methods to get probe devices. */

private:
    /* Make this class non copyable */
    CompressDeviceFactory(const CompressDeviceFactory &) = delete;
    CompressDeviceFactory &operator=(const CompressDeviceFactory &) = delete;
};
}
}
}

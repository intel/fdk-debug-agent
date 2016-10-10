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

#include "Util/Buffer.hpp"
#include "Util/Exception.hpp"
#include "cAVS/Linux/CompressTypes.hpp"
#include <string>
#include <inttypes.h>
#include <stdexcept>

namespace debug_agent
{
namespace cavs
{
namespace linux
{

enum class Mode
{
    Blocking,
    NonBlocking
};

/** This class abstracts a compress device
 */
class CompressDevice
{
public:
    using Exception = util::Exception<CompressDevice>;
    struct IoException : public Exception
    {
        /* No message required */
        IoException() : Exception("") {}
    };

    /** @throw Device::Exception if the device initialization has failed */
    CompressDevice(const compress::DeviceInfo &info) : mInfo(info), mConfig(compress::Config(0, 0))
    {
    }
    virtual ~CompressDevice() = default;

    /** below are pure virtual function of compress Device interface */
    /** Open a compress device according to the selected mode for the given role and with
     * the given configuration.
     * @param[in] mode of opening (Blocking or non blocking)
     * @param[in] role of the device (capture or playback)
     * @param[in] configuration of the device to use (buffering mode, codec specifications, ...)
     */
    virtual void open(Mode mode, compress::Role role, compress::Config &config) = 0;

    /** @return true if the compress device is running, false otherwise.
     */
    virtual bool isRunning() const noexcept = 0;

    /** @return true if the compress device is ready to use, false otherwise.
     */
    virtual bool isReady() const noexcept = 0;

    /** Close a compress device
     */
    virtual void close() noexcept = 0;

    /** Waits for either data available for write or for read according to the selection role
     * of the device.
     * @throw CompressDevice::Exception if the poll error (other than timeout)
     * @param[in] maxWaitMs max time to wait for available data before returning
     * @return true if data available, false if exiting with timeout reason
     */
    virtual bool wait(int timeoutMs = mMaxPollWaitMs) = 0;

    /** start the compress device
     */
    virtual void start() = 0;

    /** start the compress device
     */
    virtual void stop() = 0;

    /** write in the selected mode (blocking or not) the given input buffer to the compress device.
     * @param[in] inputBuffer to be written
     * @return size in bytes of data written to the device.
     */
    virtual size_t write(const util::Buffer &inputBuffer) = 0;

    /** read in the selected mode (blocking or not) to the given output buffer from the device.
     * @param[in] outputBuffer to read to
     * @return size in bytes of data read from the device.
     */
    virtual size_t read(util::Buffer &outputBuffer) = 0;

    /** Get available samples:
     * @return for write operation: the empty samples ready to be written by application
     *         for read operation: the samples ready to be read by application
     */
    virtual std::size_t getAvailable() = 0;

    const std::string getName() const { return mInfo.name(); }
    unsigned int cardId() const { return mInfo.cardId(); }
    unsigned int deviceId() const { return mInfo.deviceId(); }

    void setConfig(const compress::Config &config) { mConfig = config; }
    virtual std::size_t getBufferSize() const { return mConfig.getBufferSize(); }

    static const int mInfiniteTimeout = -1;

private:
    static const int mMaxPollWaitMs = 500;

    CompressDevice(const CompressDevice &) = delete;
    CompressDevice &operator=(const CompressDevice &) = delete;

    compress::DeviceInfo mInfo;
    compress::Config mConfig;
};
}
}
}

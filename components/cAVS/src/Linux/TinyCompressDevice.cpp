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

#include "cAVS/Linux/TinyCompressDevice.hpp"
#include "Util/convert.hpp"
#include <string.h>
#include <iostream>
#include <cassert>

using namespace debug_agent::util;
using namespace std;

namespace debug_agent
{
namespace cavs
{
namespace linux
{

unsigned int TinyCompressDevice::translateRole(compress::Role role)
{
    switch (role) {
    case compress::Role::Capture:
        return COMPRESS_OUT;
    case compress::Role::Playback:
        return COMPRESS_IN;
    }
    throw Exception("Don't know how to convert role " +
                    std::to_string(static_cast<unsigned int>(role)) +
                    " into tinycompress role type.");
}

void TinyCompressDevice::open(Mode mode, compress::Role role, compress::Config &config)
{
    CompressDevice::setConfig(config);
    mDevice = compress_open(cardId(), deviceId(), translateRole(role), config.getConfig());
    if (!mDevice || !is_compress_ready(mDevice)) {
        throw Exception("Unable to open Compress device " + getName() + ", error=" +
                        compress_get_error(mDevice));
    };
    compress_nonblock(mDevice, mode == Mode::NonBlocking);
}

void TinyCompressDevice::close() noexcept
{
    compress_close(mDevice);
}

bool TinyCompressDevice::isRunning() const noexcept
{
    return mDevice != nullptr && is_compress_running(mDevice);
}

bool TinyCompressDevice::isReady() const noexcept
{
    return mDevice != nullptr && is_compress_ready(mDevice);
}

bool TinyCompressDevice::wait(int timeoutMs)
{
    int retryMax = 10;
    while (retryMax-- > 0) {
        if (compress_wait(mDevice, timeoutMs) == 0) {
            return true;
        }
        if (errno == ETIME) {
            std::cout << "Warning: compress wait poll timed out" << std::endl;
            return false;
        }
        if (errno == EIO) {
            throw IoException();
        }
        std::cout << "Error: compress wait error: " << compress_get_error(mDevice)
                  << ", trying to recover" << std::endl;
        recover();
    }
    throw Exception("Unrecoverable error on " + getName() + ", error:" + std::to_string(errno) +
                    ", compress error=" + std::string(compress_get_error(mDevice)));
}

void TinyCompressDevice::start()
{
    assert(mDevice != nullptr);
    if (compress_start(mDevice) != 0) {
        throw Exception("Failed to start " + getName() + ", error:" +
                        std::string(compress_get_error(mDevice)));
    }
}

void TinyCompressDevice::stop()
{
    assert(mDevice != nullptr);
    if (compress_stop(mDevice) != 0) {
        throw Exception("Failed to stop: " + getName() + ", error:" +
                        std::string(compress_get_error(mDevice)));
    }
}

size_t TinyCompressDevice::write(const util::Buffer &inputBuffer)
{
    int ret = compress_write(mDevice, (const void *)inputBuffer.data(), inputBuffer.size());
    if (ret < 0) {
        throw CompressDevice::Exception("Error writing from device: " + getName() + ", error:" +
                                        std::string(compress_get_error(mDevice)));
    }
    if ((ret >= 0) && (ret < static_cast<int>(inputBuffer.size()))) {
        std::cout << "overrun detected: wrote less than expected: " << ret << "bytes." << std::endl;
    }
    return ret;
}

size_t TinyCompressDevice::read(util::Buffer &outputBuffer)
{
    int ret = compress_read(mDevice, (void *)outputBuffer.data(), outputBuffer.size());
    if (ret < 0) {
        throw CompressDevice::Exception("Error reading from device: " + getName() + ", error:" +
                                        std::string(compress_get_error(mDevice)));
    }
    if ((ret >= 0) && (ret < static_cast<int>(outputBuffer.size()))) {
        std::cout << "underrun detected: read less than expected: " << ret << "bytes." << std::endl;
        outputBuffer.resize(ret);
    }
    return ret;
}

std::size_t TinyCompressDevice::getAvailable()
{
    assert(mDevice != nullptr);
    unsigned int available;
    int ret = compress_get_avail(mDevice, &available);
    if (ret < 0) {
        throw CompressDevice::Exception("Error getting available samples from device: " +
                                        getName() + ", error:" +
                                        std::string(compress_get_error(mDevice)));
    }
    return available;
}

void TinyCompressDevice::recover()
{
    stop();
    start();
}
}
}
}

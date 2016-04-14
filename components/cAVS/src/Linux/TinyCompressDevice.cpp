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
        } else {
            std::cout << "Error: compress wait error: " << compress_get_error(mDevice)
                      << ", trying to recover" << std::endl;
            recover();
        }
    }
    throw Exception("Failed to wait/recover " + getName() + ", error:" + std::to_string(errno) +
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
    struct timespec tstamp;
    int ret = compress_get_hpointer(mDevice, &available, &tstamp);
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

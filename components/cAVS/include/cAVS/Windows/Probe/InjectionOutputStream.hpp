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

#include "cAVS/Windows/EventHandle.hpp"
#include "Util/RingBufferWriter.hpp"
#include "Util/RingBuffer.hpp"
#include "Util/Buffer.hpp"
#include "Util/Stream.hpp"
#include <algorithm>

namespace debug_agent
{
namespace cavs
{
namespace windows
{
namespace probe
{

/**
 * The injector is an active object that forwards data from a input ring buffer (feed by the http
 * layer) to an output ring buffer(consumed by the driver).
 *
 * If the input ring buffer is empty, silence is written to the driver ring buffer. To achieve
 * it, the injector knows the audio format sample byte size in order to not break the sample
 * alignment.
 */
class InjectionOutputStream final : public util::RingBufferOutputStream
{
public:
    using Exception = util::Exception<InjectionOutputStream>;

    /**
     * @param[in,out] handle A windows event handle to know when the ring buffer should be filled
     * @param[in,out] ringBufferWriter The driver probe injection ring buffer
     */
    InjectionOutputStream(EventHandle &handle, util::RingBufferWriter &&ringBufferWriter)
        : mHandleWaiter(handle), mOutputRingBuffer(std::move(ringBufferWriter))
    {
    }

    ~InjectionOutputStream() override { close(); }

    /** Close the stream, leading to unblock the thread that is writing */
    void close() { mHandleWaiter.stopWait(); }

    std::size_t getSize() const override { return mOutputRingBuffer.getSize(); }

    std::size_t getAvailable() override { return mOutputRingBuffer.getAvailableProduction(); }

    bool wait() override { return mHandleWaiter.wait(); }

    void write(const util::StreamByte *src, std::size_t byteCount) override
    {
        // writing the temporary buffer to the output ring buffer
        mOutputRingBuffer.unsafeWrite({src, src + byteCount});
    }

private:
    EventHandle::Waiter mHandleWaiter;
    util::RingBufferWriter mOutputRingBuffer;
};
}
}
}
}

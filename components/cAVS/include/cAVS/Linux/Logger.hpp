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
#pragma once

#include "cAVS/Linux/Device.hpp"
#include "cAVS/Linux/ControlDevice.hpp"
#include "cAVS/Linux/ControlDeviceTypes.hpp"
#include "cAVS/Linux/CompressTypes.hpp"
#include "cAVS/Linux/CompressDeviceFactory.hpp"
#include "cAVS/Linux/CorePower.hpp"
#include <cAVS/Logger.hpp>
#include "Util/BlockingQueue.hpp"
#include <list>
#include <mutex>
#include <future>
#include <condition_variable>

namespace debug_agent
{
namespace cavs
{
namespace linux
{

/**
 * Implements the cavs::Logger interface for Linux cAVS driver API.
 */
class Logger final : public cavs::Logger
{
public:
    Logger(Device &device, ControlDevice &controlDevice,
           CompressDeviceFactory &compressDeviceFactory)
        : mDevice(device), mControlDevice(controlDevice),
          mCompressDeviceFactory(compressDeviceFactory),
          mLogEntryQueue(queueMaxMemoryBytes, logBlockSize)
    {
    }

    ~Logger()
    {
        /* Ensure that internal threads are stopped and all consumer threads are unblocked */
        stop();
    }

    void setParameters(const Parameters &parameters) override;

    Parameters getParameters() override;

    std::unique_ptr<LogBlock> readLogBlock() override;
    void stop() noexcept override;

private:
    using BlockingLogQueue = util::BlockingQueue<LogBlock>;
    using LogBlockPtr = std::unique_ptr<LogBlock>;

    /** This class handles the log producer thread */
    class LogProducer
    {
    public:
        /* The constructor starts the log producer thread */
        LogProducer(BlockingLogQueue &queue, unsigned int coreId, Device &device,
                    std::unique_ptr<CompressDevice> logDevice);

        /** The destructor stops (and joins) the log producer thread */
        ~LogProducer();

        /** Fragment size is aligned with FW ping-ping buffer size. */
        static const size_t fragmentSize = 2048;
        /** Even if we could work with 2 fragments at driver side (tensed with FW ping pong buffer),
         * keep some margin to avoid xrun events. */
        static const size_t nbFragments = 16;

    private:
        static const unsigned int maxCommandQueueSize = 10;
        static const unsigned int maxPollWaitMs = 500;

        LogProducer(const LogProducer &) = delete;
        LogProducer &operator=(const LogProducer &) = delete;

        /** Before launching the log production thread, this operation that may throw an exception
         * is called to start the log device.
         */
        void startLogDevice();
        /** Once the log production thread has joined, this operation is called to stop the log
         * device.
         */
        void stopLogDevice();

        /** Method called by the log producer thread */
        void produceEntries();

        /** All log producers have a reference on the logger blocking queue that allows to
         * concatenate the production of log of all the DSP cores.
         */
        BlockingLogQueue &mQueue;

        /**
         * Each compress device for logging is providing to a given DSP core.
         * mCoreId stores the ID of the core corresponding to this device.
         */
        unsigned int mCoreId;

        /** Logging is produced by a compress device. */
        std::unique_ptr<CompressDevice> mLogDevice;
        std::condition_variable mCondVar;
        std::mutex mLogDeviceMutex;
        bool mProductionThreadBlocked = false;
        Device &mDevice;
        CorePower<Exception> mCorePower;
        std::future<void> mLogResult;
    };
    /** A non empty producer list garantees that we could open and start the log device.
     */
    bool isLogProductionRunning() const { return !mLogProducers.empty(); }
    static std::size_t logBlockSize(const LogBlock &block) { return block.getLogSize(); }
    static const std::size_t queueMaxMemoryBytes =
        LogProducer::nbFragments * LogProducer::fragmentSize * 320; /* 10 meg. */

    void startLogLocked(const Parameters &parameters);
    void stopLogLocked(const Parameters &parameters);
    void updateLogLocked(const Parameters &parameters);
    void constructProducers();
    void destroyProducers();

    void setLogLevel(const Level &level);
    Level getLogLevel() const;

    static mixer_ctl::LogPriority toLinux(const Level &level);
    static Level fromLinux(const mixer_ctl::LogPriority &level);

    Device &mDevice;
    ControlDevice &mControlDevice;

    CompressDeviceFactory &mCompressDeviceFactory;
    BlockingLogQueue mLogEntryQueue;

    std::list<std::unique_ptr<LogProducer>> mLogProducers;
    std::mutex mLogActivationContextMutex;
};
}
}
}

/*
********************************************************************************
*                              INTEL CONFIDENTIAL
*   Copyright(C) 2015-2016 Intel Corporation. All Rights Reserved.
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

#include "cAVS/Linux/Device.hpp"
#include "cAVS/Linux/DriverTypes.hpp"
#include "cAVS/Linux/ControlDevice.hpp"
#include "cAVS/Linux/ControlDeviceTypes.hpp"
#include "cAVS/Linux/CompressTypes.hpp"
#include "cAVS/Linux/CompressDeviceFactory.hpp"
#include <cAVS/Logger.hpp>
#include "Util/BlockingQueue.hpp"
#include <list>
#include <mutex>
#include <thread>

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
          mCompressDeviceFactory(compressDeviceFactory), mLogLevel(Level::Verbose), mCoreMask(0),
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
    void setLogStateInfo(const driver::LogStateInfo &logStateInfo);

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
        /** Allow the DSP core associated with the log producer to sleep. */
        void allowCoreToSleep() noexcept;
        void preventCoreFromSleeping();

        static const unsigned int maxCommandQueueSize = 10;
        static const unsigned int maxPollWaitMs = 500;

        /** Command to interact with LogProducer thread.
         * Only stop supported until now, may be used to drain, flush...
         */
        enum class Command : uint32_t
        {
            Exit
        };
        using CommandPtr = std::unique_ptr<Command>;
        static std::size_t commandSize(const Command &) { return sizeof(uint32_t); }
        using BlockingCommandQueue = util::BlockingQueue<Command>;
        BlockingCommandQueue mCommandQueue;

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

        /** Method to communicate with the log producer thread */
        void stopProducerThread() { sendCommand(std::make_unique<Command>(Command::Exit)); }
        void sendCommand(CommandPtr cmd);

        std::thread mProducerThread;

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

        Device &mDevice;
    };
    /** A non empty producer list garantees that we could open and start the log device.
     */
    bool isLogProductionRunning() const { return !mLogProducers.empty(); }
    static std::size_t logBlockSize(const LogBlock &block) { return block.getLogSize(); }
    static const std::size_t queueMaxMemoryBytes =
        LogProducer::nbFragments * LogProducer::fragmentSize * 320; /* 10 meg. */

    void startLogLocked(const Parameters &parameters);
    void stopLogLocked(const Parameters &parameters);
    void constructProducers();
    void destroyProducers();

    Device &mDevice;
    ControlDevice &mControlDevice;

    CompressDeviceFactory &mCompressDeviceFactory;
    Level mLogLevel;            /** Log State Info IPC is a write only, need to cache log level.  */
    driver::CoreMask mCoreMask; /** Active Core for logging. */
    BlockingLogQueue mLogEntryQueue;

    std::list<std::unique_ptr<LogProducer>> mLogProducers;
    std::mutex mLogActivationContextMutex;
};
}
}
}

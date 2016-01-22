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
#pragma once

#include "cAVS/Windows/DriverTypes.hpp"
#include "cAVS/Windows/Device.hpp"
#include "cAVS/Windows/WppClientFactory.hpp"
#include "Util/BlockingQueue.hpp"
#include "cAVS/Logger.hpp"
#include <mutex>
#include <thread>

namespace debug_agent
{
namespace cavs
{
namespace windows
{

/**
 * Implements the cavs::Logger interface for Windows cAVS driver API.
 */
class Logger final : public cavs::Logger
{
public:
    Logger(Device &device, WppClientFactory &wppClientFactory)
        : mDevice(device), mWppClientFactory(wppClientFactory),
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
    /** Maximum size of the log entry queue */
    static const std::size_t queueMaxMemoryBytes = 10 * 1024 * 1024; /* 10 meg */

    enum class Direction
    {
        Get,
        Set
    };

    using BlockingQueue = util::BlockingQueue<LogBlock>;

    /** This class handles the log producer thread */
    class LogProducer final : public windows::WppLogEntryListener
    {
    public:
        /* The constructor starts the log producer thread */
        LogProducer(BlockingQueue &queue, std::unique_ptr<WppClient> wppClient);

        /** The destructor stops (and joins) the log producer thread */
        ~LogProducer();

    private:
        LogProducer(const LogProducer &) = delete;
        LogProducer &operator=(const LogProducer &) = delete;

        /** Method called by the log producer thread */
        void produceEntries();

        /** Implements WppLogEntryListener interface */
        virtual void onLogEntry(uint32_t coreId, uint8_t *buffer, uint32_t bufferSize) override;

        BlockingQueue &mQueue;
        std::unique_ptr<WppClient> mWppClient;
        std::thread mProducerThread;
    };

    /** Translate log state to driver type */
    static driver::IOCTL_LOG_STATE translateToDriver(bool isStarted);

    /** Translate log level to driver type */
    static driver::FW_LOG_LEVEL translateToDriver(Level level);

    /** Translate log output to driver type */
    static driver::FW_LOG_OUTPUT translateToDriver(Output output);

    /** Translate log state from driver type */
    static bool translateFromDriver(driver::IOCTL_LOG_STATE state);

    /** Translate log level from driver type */
    static Level translateFromDriver(driver::FW_LOG_LEVEL level);

    /** Translate log output from driver type */
    static Output translateFromDriver(driver::FW_LOG_OUTPUT output);

    /** Translate log parameters to driver type */
    static driver::IoctlFwLogsState translateToDriver(const Parameters &params);

    /** Translate log parameters from driver type */
    static Parameters translateFromDriver(const driver::IoctlFwLogsState &params);

    /* Returns the size of a log block.
     * This method is used by the blocking queue in order to estimate the
     * memory size of all contained log blocks.
     */
    static std::size_t logBlockSize(const LogBlock &block) { return block.getLogSize(); }

    /** Set/Get log parameters using a Tiny(Get|Set) ioctl */
    void logParameterIoctl(Direction type, const driver::IoctlFwLogsState &inputFwParams,
                           driver::IoctlFwLogsState &outputFwParams);

    void setLogParameterIoctl(const Parameters &parameters);
    void startLogLocked(const Parameters &parameters);
    void stopLogLocked(const Parameters &parameters);
    void updateLogLocked(const Parameters &parameters);

    Device &mDevice;
    WppClientFactory &mWppClientFactory;
    BlockingQueue mLogEntryQueue;
    std::unique_ptr<LogProducer> mLogProducer;
    std::mutex mLogActivationContextMutex;
};
}
}
}

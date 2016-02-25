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

#include "cAVS/Driver.hpp"
#include "cAVS/DriverFactory.hpp"
#include "cAVS/Topology.hpp"
#include "cAVS/Prober.hpp"
#include "cAVS/ProbeService.hpp"
#include "Util/WrappedRaw.hpp"
#include <memory>
#include <stdexcept>
#include <vector>
#include <mutex>

namespace debug_agent
{
namespace cavs
{

/**
 * The cAVS System
 */
class System final
{
public:
    struct Exception : std::logic_error
    {
        using std::logic_error::logic_error;
    };

    /** Exclusive stream resource (input/output) */
    class StreamResource
    {
    public:
        virtual ~StreamResource() = default;

    protected:
        StreamResource(std::mutex &resourceMutex) : mLocker(resourceMutex, std::defer_lock) {}

    private:
        friend class System;

        /* Called by the System class only */
        bool tryLock() { return mLocker.try_lock(); }

        std::unique_lock<std::mutex> mLocker;
    };

    /** Exclusive output stream resource */
    class OutputStreamResource : public StreamResource
    {
    public:
        using StreamResource::StreamResource;

        /**
         * Perform writing to the supplied output stream.
         * It blocks until task is finished. For instance the log stream resource will block until
         * log capture is stopped.
         */
        virtual void doWriting(std::ostream &os) = 0;
    };

    /**
     * @throw System::Exception
     */
    System(const DriverFactory &driverFactory);

    /**
     * Set log parameters
     * @param[in] parameters Log parameters to be set
     * @throw System::Exception
     */
    void setLogParameters(Logger::Parameters &parameters);

    /**
     * Get log parameters
     * @return current log parameters
     * @throw System::Exception
     */
    Logger::Parameters getLogParameters();

    /**
     * Get module entries
     */
    const std::vector<dsp_fw::ModuleEntry> &getModuleEntries() const noexcept;

    /**
    * Find module entry by its id
    */
    const dsp_fw::ModuleEntry &findModuleEntry(uint16_t moduleId) const;

    /**
    * Find module entry by its name
    */
    const dsp_fw::ModuleEntry &findModuleEntry(const std::string &name) const;

    /**
     * Get firmware configuration
     */
    const dsp_fw::FwConfig &getFwConfig() const noexcept;

    /**
     * Get hardware configuration
     */
    const dsp_fw::HwConfig &getHwConfig() const noexcept;

    /**
     * Try to acquire the log stream resource
     *
     * The resource will be locked until the returned OutputStreamResource instance is released.
     *
     * @return a OutputStreamResource instance if the locking is successful, otherwise nullptr.
     */
    std::unique_ptr<OutputStreamResource> tryToAcquireLogStreamResource();

    /** Set module parameter */
    void setModuleParameter(uint16_t moduleId, uint16_t instanceId, dsp_fw::ParameterId parameterId,
                            const util::Buffer &parameterPayload);

    /** Get module parameter */
    void getModuleParameter(uint16_t moduleId, uint16_t instanceId, dsp_fw::ParameterId parameterId,
                            util::Buffer &parameterPayload);

    /** @return topology */
    void getTopology(Topology &topology);

    /** Set the state of the probing service.
     * @throw System::Exception
     */
    void setProberState(bool active);

    /**
     * Get the state of the probing service
     */
    bool isProberActive();

    /** Set probe configuration. This configuration will be used at next probe service start.
     * @throw System::Exception
     */
    void setProbeConfiguration(ProbeId probeId, const Prober::ProbeConfig &probeCfg);

    /**
    * @throw System::Exception
    */
    Prober::ProbeConfig getProbeConfiguration(ProbeId probeId);

    /** Stop internal threads and unblock consumer threads */
    void stop() noexcept { mDriver->stop(); }

private:
    /** Exclusive resource used to retrieve log data */
    class LogStreamResource : public OutputStreamResource
    {
    public:
        LogStreamResource(std::mutex &resourceMutex, Logger &logger,
                          const std::vector<dsp_fw::ModuleEntry> &moduleEntries)
            : OutputStreamResource(resourceMutex), mLogger(logger), mModuleEntries(moduleEntries)
        {
        }

        void doWriting(std::ostream &os) override;

    private:
        Logger &mLogger;
        const std::vector<dsp_fw::ModuleEntry> &mModuleEntries;
    };

    /* Make this class non copyable */
    System(const System &) = delete;
    System &operator=(const System &) = delete;

    /** Try to acquire resource of the supplied template parameter type
     * @return the resource if it has successfully acquired, otherwise nullptr.
     */
    template <typename T>
    std::unique_ptr<T> tryToAcquireResource(std::unique_ptr<T> resource);

    static std::unique_ptr<Driver> createDriver(const DriverFactory &driverFactory);

    std::unique_ptr<Driver> mDriver;

    /**
     * The module entries table retrieved from FW once, at initialization
     */
    std::vector<dsp_fw::ModuleEntry> mModuleEntries;

    /**
     * The FW config structure retrieved from FW once, at initialization
     */
    dsp_fw::FwConfig mFwConfig;

    /**
     * The HW config structure retrieved from FW once, at initialization
     */
    dsp_fw::HwConfig mHwConfig;

    /** Mutex that guarantees log stream exclusive usage */
    std::mutex mLogStreamMutex;

    ProbeService mProbeService;
};
}
}

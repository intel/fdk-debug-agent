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

#include "cAVS/Driver.hpp"
#include "cAVS/DriverFactory.hpp"
#include "cAVS/Topology.hpp"
#include "cAVS/Prober.hpp"
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
    class Exception : public std::logic_error
    {
    public:
        explicit Exception(const std::string &what) : std::logic_error(what) {}
    };

    /** Exclusive resource used to retrieve log data */
    class LogStreamResource
    {
    public:
        /**
         * Streams out log in IFDK:cavs:fwlog format
         *
         * @param[in] os the std::ostream where the log has to be written to
         *
         * @throw System::Exception
         */
        void doLogStream(std::ostream &os) { mSystem.doLogStreamInternal(os); }

    private:
        friend class System;

        /* Called by the System class only */
        LogStreamResource(System &system)
            : mLocker(system.mLogStreamMutex, std::defer_lock), mSystem(system)
        {
        }

        /* Called by the System class only */
        bool tryLock() { return mLocker.try_lock(); }

        std::unique_lock<std::mutex> mLocker;
        System &mSystem;
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
    * Find module entry
    */
    const dsp_fw::ModuleEntry &findModuleEntry(uint16_t moduleId) const;

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
     * The resource will be locked until the returned LogStreamResource instance is released.
     *
     * @return a LogStreamResource instance if the locking is successful, otherwise nullptr.
     */
    std::unique_ptr<LogStreamResource> tryToAcquireLogStreamResource();

    /** Set module parameter */
    void setModuleParameter(uint16_t moduleId, uint16_t instanceId, dsp_fw::ParameterId parameterId,
                            const util::Buffer &parameterPayload);

    /** Get module parameter */
    void getModuleParameter(uint16_t moduleId, uint16_t instanceId, dsp_fw::ParameterId parameterId,
                            util::Buffer &parameterPayload);

    /** @return topology */
    void getTopology(Topology &topology);

    /** Set the state of the probing service.
     * State transitions must be respected (see SwAS)
     * @throw System::Exception
     */
    void setProberState(Prober::State state);

    /**
     * Get the state of the probing service
     * @throw System::Exception if an error occurs
     */
    Prober::State getProberState();

    /** Set probes for the future session.
     *
     * Probe service state shall be 'Owned'.
     *
     * @throw System::Exception
     */
    void setSessionProbes(const std::vector<Prober::ProbeConfig> probes);

    /** Get probes for the current/future session.
     *
     * Probe service state shall be in 'Owned, Allocated, Running'.
     *
     * @throw System::Exception
     */
    std::vector<Prober::ProbeConfig> getSessionProbes();

    /** Stop internal threads and unblock consumer threads */
    void stop() noexcept { mDriver->stop(); }

private:
    /* Make this class non copyable */
    System(const System &) = delete;
    System &operator=(const System &) = delete;

    static std::unique_ptr<Driver> createDriver(const DriverFactory &driverFactory);
    void doLogStreamInternal(std::ostream &os);

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
};
}
}

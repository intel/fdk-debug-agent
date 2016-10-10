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

#include "cAVS/Driver.hpp"
#include "cAVS/DriverFactory.hpp"
#include "cAVS/Topology.hpp"
#include "cAVS/ProbeService.hpp"
#include "cAVS/Prober.hpp"
#include "cAVS/PerfService.hpp"
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

    class InputStreamResource : public StreamResource
    {
    public:
        using StreamResource::StreamResource;

        /**
        * Perform reading from the supplied input stream.
        * It blocks inputstream is empty or failed
        */
        virtual void doReading(std::istream &is) = 0;
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
     * Try to acquire the log stream resource
     *
     * The resource will be locked until the returned OutputStreamResource instance is released.
     *
     * @return a OutputStreamResource instance if the locking is successful, otherwise nullptr.
     */
    std::unique_ptr<OutputStreamResource> tryToAcquireLogStreamResource();

    /**
     * Try to acquire probe extraction stream resource
     *
     * The resource will be locked until the returned OutputStreamResource instance is released.
     *
     * @return a OutputStreamResource instance if the locking is successful, otherwise nullptr.
     */
    std::unique_ptr<OutputStreamResource> tryToAcquireProbeExtractionStreamResource(
        ProbeId probeIndex);

    /**
     * Try to acquire probe injection stream resource
     *
     * The resource will be locked until the returned InputStreamResource instance is released.
     *
     * @return a InputStreamResource instance if the locking is successful, otherwise nullptr.
     */
    std::unique_ptr<InputStreamResource> tryToAcquireProbeInjectionStreamResource(
        ProbeId probeIndex);

    /** @return topology */
    void getTopology(Topology &topology);

    ModuleHandler &getModuleHandler();
    ProbeService &getProbeService();
    PerfService &getPerfService();

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
    /** Exclusive resource used to retrieve probe extraction data */
    class ProbeExtractionStreamResource : public OutputStreamResource
    {
    public:
        ProbeExtractionStreamResource(std::mutex &resourceMutex, Prober &prober, ProbeId probeIndex)
            : OutputStreamResource(resourceMutex), mProber(prober), mProbeIndex(probeIndex)
        {
        }

        void doWriting(std::ostream &os) override;

    private:
        Prober &mProber;
        ProbeId mProbeIndex;
    };

    /** Exclusive resource used to inject data to probe */
    class ProbeInjectionStreamResource : public InputStreamResource
    {
    public:
        ProbeInjectionStreamResource(std::mutex &resourceMutex, Prober &prober, ProbeId probeIndex)
            : InputStreamResource(resourceMutex), mProber(prober), mProbeIndex(probeIndex)
        {
        }

        void doReading(std::istream &os) override;

    private:
        Prober &mProber;
        ProbeId mProbeIndex;
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

    /** Mutex that guarantees log stream exclusive usage */
    std::mutex mLogStreamMutex;

    ProbeService mProbeService;
    /** Mutexes that guarantee probe stream exclusive usage */
    std::vector<std::mutex> mProbeExtractionMutexes;
    std::vector<std::mutex> mProbeInjectionMutexes;

    PerfService mPerfService;
};
}
}

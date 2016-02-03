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

#include "Util/Buffer.hpp"
#include <inttypes.h>
#include <stdexcept>
#include <string>
#include <vector>
#include <memory>

namespace debug_agent
{
namespace cavs
{

/**
 * This interface intends to abstract probing OS-specificities
 *
 * An important choice has been made: this interface lets the os-specific implementation decide
 * and implement its own threading model (the threading model that uses the driver and
 * fills/consumes audio block queues).
 *
 * This choice has been made because Linux driver api (based on tiny-alsa) and windows api (based
 * on ioctl) are very heterogeneous.
 *
 * The drawback is that the os-specific logic is larger and more complex
 *
 * @todo: write usage sequence diagram
 *
 */
class Prober
{
public:
    struct Exception : std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };

    /** State of the probing service */
    enum class State
    {
        Idle,
        Owned,     /// probe service is owned (required because it's a monoclient service)
        Allocated, /// Buffers are allocated
        Active     /// Probing is running
    };

    enum class ProbeType
    {
        Input,
        Output,
        Internal
    };

    enum class ProbePurpose
    {
        Inject,
        Extract,
        InjectReextract
    };

    /** Identify a probe point into the topology */
    struct ProbePointId
    {
        uint32_t moduleTypeId;
        uint32_t moduleInstanceId;
        ProbeType type;
        uint32_t pinIndex;

        ProbePointId(uint32_t moduleTypeId, uint32_t moduleInstanceId, ProbeType type,
                     uint32_t pinIndex)
            : moduleTypeId(moduleTypeId), moduleInstanceId(moduleInstanceId), type(type),
              pinIndex(pinIndex)
        {
        }

        bool operator==(const ProbePointId &other) const
        {
            return moduleTypeId == other.moduleTypeId &&
                   moduleInstanceId == other.moduleInstanceId && type == other.type &&
                   pinIndex == other.pinIndex;
        }
    };

    /** Configuration of one probe instance */
    struct ProbeConfig
    {
        ProbePointId id;      /* The probe point id where the probe will be connected */
        ProbePurpose purpose; /* The probe purpose (inject, extract, both) */

        ProbeConfig(const ProbePointId &id, ProbePurpose purpose) : id(id), purpose(purpose) {}

        bool operator==(const ProbeConfig &other) const
        {
            return id == other.id && purpose == other.purpose;
        }
    };

    Prober() = default;

    virtual ~Prober() = default;

    /** Set the state of the probing service.
     *
     * The state machine specified in the SwAS must be respected.
     *
     * @todo draw state machine
     * @throw Prober::Exception
     */
    virtual void setState(State state) = 0;

    /**
     * Get the state of the probing service
     *
     * @throw Prober::Exception
     */
    virtual State getState() = 0;

    /** Set probes for the future session.
     *
     * Probe service state shall be 'Owned'.
     *
     * @throw Prober::Exception
     */
    virtual void setSessionProbes(const std::vector<ProbeConfig> probes) = 0;

    /** Get probes for the current/future session.
     *
     * Probe service state shall be in 'Owned, Allocated, Active'.
     *
     * @throw Prober::Exception
     */
    virtual std::vector<ProbeConfig> getSessionProbes() = 0;

    /**
     * Return the next extraction block data
     *
     * This method blocks until data is available (or probe is stopped)
     *
     * Probe service state shall be in 'Active'.
     *
     * @param[in] probeIndex the index of the probe to query
     * @return the next extraction block data, or nullptr if the probe has been stopped.
     *
     * @throw Prober::Exception
     */
    virtual std::unique_ptr<util::Buffer> dequeueExtractionBlock(uint32_t probeIndex) = 0;

    /**
     * Enqueue a block that will be injected to the probe.
     *
     * Probe service state shall be in 'Active'.
     *
     * @param[in] probeIndex the index of the probe to query
     * @param[out] buffer the block to inject
     * @return true if the block has been enqueued, of false if the probe is closed.
     *
     * @throw Prober::Exception
     */
    virtual bool enqueueInjectionBlock(uint32_t probeIndex, const util::Buffer &buffer) = 0;
};
}
}

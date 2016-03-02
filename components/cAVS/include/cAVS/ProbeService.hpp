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

#include "cAVS/Prober.hpp"
#include <vector>
#include <mutex>
#include <map>

namespace debug_agent
{
namespace cavs
{

/** This class pilots the driver probe service
 *
 * The client changes only a simple boolean state (active, stopped). Changing this state
 * leads to set driver probe service state according the state machine specified in the SwAS.
 * Note: Driver probe service states are (idle, owned, allocated, active)
 *
 * By choice, the high level state (active, stopped) is not stored in this class, but retrieved
 * from driver. This helps to avoid divergent states.
 *
 * The public API of this class is thread safe.
 */
class ProbeService
{
public:
    struct Exception : std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };

    ProbeService(Prober &prober) : mProbeConfigs(prober.getMaxProbeCount()), mProber(prober) {}
    ~ProbeService();

    /** Set service state
     * @throw ProbeService::Exception
     */
    void setState(bool active);

    /** Get service state
     * @throw ProbeService::Exception
     */
    bool isActive();

    /** Set configuration of one probe
     * @throw ProbeService::Exception if the probe id is wrong
     */
    void setProbeConfig(ProbeId id, const Prober::ProbeConfig &config);

    /** Get configuration of one probe
     * @throw ProbeService::Exception if the probe id is wrong
     */
    Prober::ProbeConfig getProbeConfig(ProbeId id) const;

private:
    ProbeService(const ProbeService &) = delete;
    ProbeService &operator=(const ProbeService &) = delete;

    using Transitions = std::map<Prober::State, Prober::State>;

    /* Check if probe id is in valid range */
    void checkProbeId(ProbeId id) const;

    /**
    * @throw ProbeService::Exception
    */
    void start();

    /**
    * @throw ProbeService::Exception
    */
    void stop();

    void stopNoThrow() noexcept;

    /** Go to a target state using supplied transitions
     * @throw ProbeService::Exception
     */
    void goToState(Prober::State targetState, const Transitions &transitions, bool starting);

    /** Process state-specific logic (for instance, the 'Owned' state requires probe configuration
     * @throw ProbeService::Exception
     */
    void processState(Prober::State state, bool starting);

    /**
     * @throw ProbeService::Exception
     */
    Prober::State checkAndGetStateFromDriver();

    /**
     * @throw ProbeService::Exception
     */
    void setStateToDriver(Prober::State state);

    /** State machine transitions leading to "Active" state from any other state*/
    static const Transitions mStartTransitions;

    /** State machine transitions leading to "Idle" state from any other state */
    static const Transitions mStopTransitions;

    std::vector<Prober::ProbeConfig> mProbeConfigs;
    Prober &mProber;
    mutable std::mutex mDriverLogServiceStateMutex;
    mutable std::mutex mProbeConfigMutex;
};
}
}

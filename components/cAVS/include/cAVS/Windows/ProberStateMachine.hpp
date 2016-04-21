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

#include "cAVS/Prober.hpp"
#include "cAVS/Windows/ProberBackend.hpp"

#include <Util/Exception.hpp>

#include <mutex>

namespace debug_agent
{
namespace cavs
{
namespace windows
{

/** This class pilots the driver probe state machine
 *
 * The client changes only a simple boolean state (active, stopped). Changing this state
 * leads to set driver probe service state according the state machine specified in the SwAS.
 * Note: Driver probe service states are (idle, owned, allocated, active)
 *
 * By choice, the high level state (active, stopped) is not stored in this class, but retrieved
 * from driver. This helps to avoid divergent states.
 */
class ProberStateMachine
{
public:
    using Exception = util::Exception<ProberStateMachine>;

    ProberStateMachine(ProberBackend &proberBackend) : mProberBackend(proberBackend) {}
    ProberStateMachine() = default;

    void setState(bool active);

    bool isActive();

    void stopNoThrow() noexcept;

private:
    ProberStateMachine(const ProberStateMachine &) = delete;
    ProberStateMachine &operator=(const ProberStateMachine &) = delete;

    using Transitions = std::map<ProberBackend::State, ProberBackend::State>;

    /**
    * @throw Prober::Exception
    */
    void start();

    /**
    * @throw Prober::Exception
    */
    void stop();

    /** Go to a target state using supplied transitions
     * @throw Prober::Exception
     */
    void goToState(ProberBackend::State targetState, const Transitions &transitions);

    /**
     * @throw Prober::Exception
     */
    ProberBackend::State checkAndGetStateFromDriver();

    /**
     * @throw Prober::Exception
     */
    void setStateTransitionToDriver(ProberBackend::State newState, ProberBackend::State oldState);

    /** State machine transitions leading to "Active" state from any other state*/
    static const Transitions mStartTransitions;

    /** State machine transitions leading to "Idle" state from any other state */
    static const Transitions mStopTransitions;

    ProberBackend &mProberBackend;

    std::mutex mStateMutex;
};
}
}
}

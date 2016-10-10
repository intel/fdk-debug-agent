/*
 * Copyright (c) 2016, Intel Corporation
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

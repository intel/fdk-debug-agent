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
#include "cAVS/Windows/ProberStateMachine.hpp"
#include <Util/AssertAlways.hpp>
#include <iostream>

namespace debug_agent
{
namespace cavs
{
namespace windows
{

const ProberStateMachine::Transitions ProberStateMachine::mStartTransitions{
    {ProberBackend::State::Idle, ProberBackend::State::Owned}, /* Frome Idle, must go to Owned */
    {ProberBackend::State::Owned,
     ProberBackend::State::Allocated}, /* Frome Owned, must go to Allocated */
    {ProberBackend::State::Allocated, ProberBackend::State::Active}, /* And so on.. */
};

const ProberStateMachine::Transitions ProberStateMachine::mStopTransitions{
    {ProberBackend::State::Owned, ProberBackend::State::Idle},
    {ProberBackend::State::Allocated, ProberBackend::State::Owned},
    {ProberBackend::State::Active, ProberBackend::State::Allocated},
};

void ProberStateMachine::setState(bool active)
{
    std::lock_guard<std::mutex> guard(mStateMutex);

    if (active) {
        try {
            start();
        } catch (Exception &) {
            /* Trying to stop if starting has failed to come back to a consistent state */
            stopNoThrow();
            throw;
        }
    } else {
        stop();
    }
}

bool ProberStateMachine::isActive()
{
    std::lock_guard<std::mutex> guard(mStateMutex);

    ProberBackend::State currentState = checkAndGetStateFromDriver();
    switch (currentState) {
    case ProberBackend::State::Idle:
        return false;
    case ProberBackend::State::Active:
        return true;
    case ProberBackend::State::Allocated:
    case ProberBackend::State::Owned:
        /* Inconsistent state : throwing */
        throw Exception("Unexpected driver probe service state: " +
                        ProberBackend::stateHelper().toString(currentState));
    }

    /* State has already been checked */
    ASSERT_ALWAYS(false);
}

void ProberStateMachine::start()
{
    goToState(ProberBackend::State::Active, mStartTransitions);
}

void ProberStateMachine::stop()
{
    goToState(ProberBackend::State::Idle, mStopTransitions);
}

void ProberStateMachine::stopNoThrow() noexcept
{
    try {
        stop();
    } catch (Exception &e) {
        /* @todo : use log */
        std::cout << "Unable to stop driver probe service: " << e.what() << "\n";
    }
}

void ProberStateMachine::goToState(ProberBackend::State targetState, const Transitions &transitions)
{
    ProberBackend::State currentState = checkAndGetStateFromDriver();

    while (currentState != targetState) {
        auto it = transitions.find(currentState);

        /* currentState is valid, so it must be found in the map*/
        ASSERT_ALWAYS(it != transitions.end());

        currentState = it->second;

        setStateTransitionToDriver(currentState, it->first);
    }
}

ProberBackend::State ProberStateMachine::checkAndGetStateFromDriver()
{
    ProberBackend::State state;
    try {
        state = mProberBackend.getState();
    } catch (ProberBackend::Exception &e) {
        throw Exception("Unable to get state from driver: " + std::string(e.what()));
    }
    if (!ProberBackend::stateHelper().isValid(state)) {
        throw Exception("Invalid state returned by driver: " +
                        std::to_string(static_cast<uint32_t>(state)));
    }
    return state;
}

void ProberStateMachine::setStateTransitionToDriver(ProberBackend::State newState,
                                                    ProberBackend::State oldState)
{
    ASSERT_ALWAYS(ProberBackend::stateHelper().isValid(newState));
    try {
        mProberBackend.setStateTransition(newState, oldState);
    } catch (ProberBackend::Exception &e) {
        throw Exception("Unable to set state to driver: " + std::string(e.what()));
    }
}
}
}
}

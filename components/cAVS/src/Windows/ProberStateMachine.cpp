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

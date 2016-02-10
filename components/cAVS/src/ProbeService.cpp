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
#include "cAVS/ProbeService.hpp"
#include <util/AssertAlways.hpp>
#include <iostream>

namespace debug_agent
{
namespace cavs
{

const ProbeService::Transitions ProbeService::mStartTransitions{
    {Prober::State::Idle, Prober::State::Owned},       /* Frome Idle, must go to Owned */
    {Prober::State::Owned, Prober::State::Allocated},  /* Frome Owned, must go to Allocated */
    {Prober::State::Allocated, Prober::State::Active}, /* And so on.. */
};

const ProbeService::Transitions ProbeService::mStopTransitions{
    {Prober::State::Owned, Prober::State::Idle},
    {Prober::State::Allocated, Prober::State::Owned},
    {Prober::State::Active, Prober::State::Allocated},
};

ProbeService::~ProbeService()
{
    stopNoThrow();
}

void ProbeService::setState(bool active)
{
    std::lock_guard<std::mutex> guard(mDriverLogServiceStateMutex);

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

bool ProbeService::isActive()
{
    std::lock_guard<std::mutex> guard(mDriverLogServiceStateMutex);

    Prober::State currentState = checkAndGetStateFromDriver();
    switch (currentState) {
    case Prober::State::Idle:
        return false;
    case Prober::State::Active:
        return true;
    case Prober::State::Allocated:
    case Prober::State::Owned:
        /* Inconsistent state : throwing */
        throw Exception("Unexpected driver probe service state: " +
                        Prober::stateHelper().toString(currentState));
    }

    /* State has already been checked */
    ASSERT_ALWAYS(false);
}

void ProbeService::start()
{
    goToState(Prober::State::Active, mStartTransitions);
}

void ProbeService::stop()
{
    goToState(Prober::State::Idle, mStopTransitions);
}

void ProbeService::stopNoThrow() noexcept
{
    try {
        stop();
    } catch (Exception &e) {
        /* @todo : use log */
        std::cout << "Unable to stop driver probe service: " << e.what() << "\n";
    }
}

void ProbeService::checkProbeId(ProbeId probeId)
{
    if (probeId.getValue() >= mProbeCount) {
        throw Exception("Invalid probe index: " + std::to_string(probeId.getValue()));
    }
}

void ProbeService::setProbeConfig(ProbeId probeId, const Prober::ProbeConfig &config)
{
    checkProbeId(probeId);

    std::lock_guard<std::mutex> guard(mProbeConfigMutex);
    mProbeConfigs[probeId.getValue()] = config;
}

Prober::ProbeConfig ProbeService::getProbeConfig(ProbeId probeId) const
{
    checkProbeId(probeId);

    std::lock_guard<std::mutex> guard(mProbeConfigMutex);
    return mProbeConfigs[probeId.getValue()];
}

void ProbeService::goToState(Prober::State targetState, const Transitions &transitions)
{
    Prober::State currentState = checkAndGetStateFromDriver();
    processState(currentState);

    while (currentState != targetState) {
        auto it = transitions.find(currentState);

        /* currentState is valid, so it must be found in the map*/
        ASSERT_ALWAYS(it != transitions.end());

        currentState = it->second;

        setStateToDriver(currentState);
        processState(currentState);
    }
}

void ProbeService::processState(Prober::State state)
{
    if (state == Prober::State::Owned) {
        /* State is "Owned" : applying cached configuration to the driver */
        try {
            std::lock_guard<std::mutex> guard(mProbeConfigMutex);
            return mProber.setSessionProbes(mProbeConfigs);
        } catch (Prober::Exception &e) {
            throw Exception("Unable to set session probes to driver: " + std::string(e.what()));
        }
    }
}

Prober::State ProbeService::checkAndGetStateFromDriver()
{
    Prober::State state;
    try {
        state = mProber.getState();
    } catch (Prober::Exception &e) {
        throw Exception("Unable to get state from driver: " + std::string(e.what()));
    }
    if (!Prober::stateHelper().isValid(state)) {
        throw Exception("Invalid state returned by driver: " +
                        std::to_string(static_cast<uint32_t>(state)));
    }
    return state;
}

void ProbeService::setStateToDriver(Prober::State state)
{
    ASSERT_ALWAYS(Prober::stateHelper().isValid(state));
    try {
        mProber.setState(state);
    } catch (Prober::Exception &e) {
        throw Exception("Unable to set state to driver: " + std::string(e.what()));
    }
}
}
}

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
#include "cAVS/Driver.hpp"
#include "Util/Exception.hpp"
#include <mutex>

namespace debug_agent
{
namespace cavs
{
/** This class handles the configuration and activation cycle of the Prober. */
class ProbeService
{
public:
    using Exception = util::Exception<ProbeService>;

    ProbeService(Driver &driver)
        : mDriver(driver), mProbeConfigs(mDriver.getProber().getMaxProbeCount())
    {
    }

    /** Set the state of the probing service.
     * @throw System::Exception
     */
    void setState(bool active);
    /**
     * Get the state of the probing service
     */
    bool isActive();
    void checkProbeId(ProbeId probeId) const;
    /** Set probe configuration. This configuration will be used at next probe service start.
     * @throw System::Exception
     */
    void setConfiguration(ProbeId id, const Prober::ProbeConfig &probe);
    /**
    * @throw System::Exception
    */
    Prober::ProbeConfig getConfiguration(ProbeId id);

    /** @return the sample byte size of each injection probes used to inject silence if
     * underrun happens.
     */
    const Prober::InjectionSampleByteSizes getInjectionSampleByteSizes() const;

private:
    Driver &mDriver;
    Prober::SessionProbes mProbeConfigs;
    mutable std::mutex mProbeConfigMutex;
};
} // cavs
} // debug_agent

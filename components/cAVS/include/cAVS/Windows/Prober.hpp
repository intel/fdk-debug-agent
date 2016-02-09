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
#include "cAVS/Windows/Device.hpp"
#include "Util/ByteStreamReader.hpp"
#include "Util/ByteStreamWriter.hpp"

namespace debug_agent
{
namespace cavs
{
namespace windows
{

class Prober : public cavs::Prober
{
public:
    Prober(Device &device) : mDevice(device) {}

    void setState(State state) override;
    State getState() override;
    void setSessionProbes(const SessionProbes probes) override;
    SessionProbes getSessionProbes() override;
    std::unique_ptr<util::Buffer> dequeueExtractionBlock(uint32_t probeIndex) override;
    bool enqueueInjectionBlock(uint32_t probeIndex, const util::Buffer &buffer) override;

private:
    template <driver::IoCtlType type, ULONG id, class T>
    struct IoctlParameter
    {
        static constexpr driver::IoCtlType type{type};
        static constexpr ULONG id{id};
        using Data = T;
    };

    // 0 = get/setState
    using GetState = IoctlParameter<driver::IoCtlType::TinyGet, 0, driver::ProbeState>;
    using SetState = IoctlParameter<driver::IoCtlType::TinySet, 0, driver::ProbeState>;
    // 1 = get/setProbePointConfiguration
    using GetProbePointConfiguration =
        IoctlParameter<driver::IoCtlType::TinyGet, 1, driver::ProbePointConfiguration>;
    using SetProbePointConfiguration =
        IoctlParameter<driver::IoCtlType::TinySet, 1, driver::ProbePointConfiguration>;

    /** Send a probes-related ioctl to the driver
     *
     * @tparam T A type describing the ioctl (id, direction, type of the data
     *           to be sent - described by a Data member).
     * @param[in,out] inout Reference to the data to be sent/received.
     */
    template <class T>
    void ioctl(typename T::Data &inout);

    static void throwIfIllegal(const ProbePointId &candidate);

    /** Convert values from OS-agnostic cAVS to cAVS Windows driver and vice-versa
     */
    /** @{ */
    static driver::ProbeState toWindows(const State &from);
    static State fromWindows(const driver::ProbeState &from);
    static driver::ProbePointId toWindows(const ProbePointId &from);
    static ProbePointId fromWindows(const driver::ProbePointId &from);
    static driver::ProbePurpose toWindows(const ProbePurpose &from);
    static ProbePurpose fromWindows(const driver::ProbePurpose &from);
    /** @} */

    Device &mDevice;
};
}
}
}

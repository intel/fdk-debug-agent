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

#include "cAVS/Linux/MockedControlDeviceCommands.hpp"
#include "cAVS/Linux/ModuleHandler.hpp"
#include "cAVS/Linux/DriverTypes.hpp"
#include "Util/Buffer.hpp"
#include "Util/ByteStreamWriter.hpp"
#include "Util/ByteStreamReader.hpp"

using namespace debug_agent::util;

namespace debug_agent
{
namespace cavs
{
namespace linux
{

void MockedControlDeviceCommands::addGetLogLevelCommand(bool controlSuccess,
                                                        mixer_ctl::LogPriority expectedLogPrio)
{
    MemoryByteStreamWriter writer;
    writer.write(static_cast<long>(expectedLogPrio));
    if (controlSuccess) {
        mControlDevice.addSuccessfulControlReadEntry(mixer_ctl::logLevelMixer, {},
                                                     writer.getBuffer());
    } else {
        mControlDevice.addFailedControlReadEntry(mixer_ctl::logLevelMixer, {}, writer.getBuffer());
    }
}

void MockedControlDeviceCommands::addSetLogLevelCommand(bool controlSuccess,
                                                        mixer_ctl::LogPriority logPrio)
{
    MemoryByteStreamWriter writer;
    writer.write(static_cast<long>(logPrio));
    mControlDevice.addControlWriteEntry(controlSuccess, mixer_ctl::logLevelMixer,
                                        writer.getBuffer());
}

void MockedControlDeviceCommands::addGetProbeControlCommand(
    bool controlSuccess, unsigned int probeIndex, mixer_ctl::ProbeControl expectedProbeControl)
{
    MemoryByteStreamWriter writer;
    writer.write(expectedProbeControl);
    if (expectedProbeControl.getPurpose() == mixer_ctl::ProbePurpose::Inject ||
        expectedProbeControl.getPurpose() == mixer_ctl::ProbePurpose::InjectReextract) {
        mControlDevice.addControlReadEntry(controlSuccess,
                                           mixer_ctl::getProbeInjectControlMixer(probeIndex), {},
                                           writer.getBuffer());
    } else {
        mControlDevice.addControlReadEntry(controlSuccess,
                                           mixer_ctl::getProbeExtractControlMixer(probeIndex), {},
                                           writer.getBuffer());
    }
}

void MockedControlDeviceCommands::addSetProbeControlCommand(bool controlSuccess,
                                                            unsigned int probeIndex,
                                                            mixer_ctl::ProbeControl probeControl)
{
    MemoryByteStreamWriter writer;
    writer.write(probeControl);
    if (probeControl.getPurpose() == mixer_ctl::ProbePurpose::Inject ||
        probeControl.getPurpose() == mixer_ctl::ProbePurpose::InjectReextract) {
        mControlDevice.addControlWriteEntry(
            controlSuccess, mixer_ctl::getProbeInjectControlMixer(probeIndex), writer.getBuffer());
    }
    if (probeControl.getPurpose() == mixer_ctl::ProbePurpose::Extract ||
        probeControl.getPurpose() == mixer_ctl::ProbePurpose::InjectReextract) {
        mControlDevice.addControlWriteEntry(
            controlSuccess, mixer_ctl::getProbeExtractControlMixer(probeIndex), writer.getBuffer());
    }
}
}
}
}

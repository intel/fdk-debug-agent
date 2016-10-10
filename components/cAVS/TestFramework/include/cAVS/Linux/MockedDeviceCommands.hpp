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

#include "cAVS/DspFw/ModuleType.hpp"
#include "cAVS/DspFw/ModuleInstance.hpp"
#include "cAVS/DspFw/FwConfig.hpp"
#include "cAVS/DspFw/HwConfig.hpp"
#include "cAVS/DspFw/Pipeline.hpp"
#include "cAVS/DspFw/Gateway.hpp"
#include "cAVS/DspFw/Scheduler.hpp"
#include "cAVS/DspFw/Infrastructure.hpp"
#include "cAVS/DspFw/GlobalPerfData.hpp"
#include "cAVS/Linux/MockedDevice.hpp"
#include "cAVS/Perf.hpp"
#include "Util/Buffer.hpp"

namespace debug_agent
{
namespace cavs
{
namespace linux
{

/** This class helps to feed a mocked device test vector by adding debugfs commands */
class MockedDeviceCommands final
{
public:
    MockedDeviceCommands(MockedDevice &device) : mDevice(device) {}

    /**
     * Add a get hw config command.
     *
     * @param[in] returnedFirmwareStatus the returned firmware status
     * @param[in] hwConfigTlvList the hw config returned by the ioctl, which is a TLV list.
     *
     * Note: the hwConfigTlvList parameter is unused if :
     * - returnedFirmwareStatus != ADSP_IPC_SUCCESS
     *
     * @throw Device::Exception
     */
    void addGetHwConfigCommand(dsp_fw::IxcStatus returnedFirmwareStatus,
                               const util::Buffer &hwConfigTlvList);

    /** Add a get pipeline props command.
     *
     * @param[in] returnedFirmwareStatus the returned firmware status
     * @param[in] pipelineId the id of the requested pipeline
     * @param[in] props the returned pipeline properties
     *
     * Note: the props parameter is unused if :
     * - returnedFirmwareStatus != ADSP_IPC_SUCCESS
     *
     * @throw Device::Exception
     */
    void addGetPipelinePropsCommand(dsp_fw::IxcStatus returnedFirmwareStatus,
                                    dsp_fw::PipeLineIdType pipelineId,
                                    const dsp_fw::PplProps &props);

    /** Add a get schedulers info command.
     *
     * @param[in] returnedFirmwareStatus the returned firmware status
     * @param[in] coreId the id of the requested core
     * @param[in] info the returned schedulers info
     *
     * Note: the info parameter is unused if :
     * - returnedFirmwareStatus != ADSP_IPC_SUCCESS
     *
     * @throw Device::Exception
     */
    void addGetSchedulersInfoCommand(dsp_fw::IxcStatus returnedFirmwareStatus,
                                     dsp_fw::CoreId coreId, const dsp_fw::SchedulersInfo &info);

    /** Add a get pipeline list command.
     *
     * @param[in] returnedFirmwareStatus the returned firmware status
     * @param[in] maxPplCount the maximum pipeline count
     * @param[in] pipelineIds the pipeline id list returned by the ioctl.
     *
     * Note: the pipelineIds parameter is unused if :
     * - returnedFirmwareStatus != ADSP_IPC_SUCCESS
     *
     * @throw Device::Exception
     */
    void addGetPipelineListCommand(dsp_fw::IxcStatus returnedFirmwareStatus, uint32_t maxPplCount,
                                   const std::vector<dsp_fw::PipeLineIdType> &pipelineIds);

    /** Add a get gateways command.
     *
     * @param[in] returnedFirmwareStatus the returned firmware status
     * @param[in] gatewayCount the gateway count
     * @param[in] gateways the gateway list returned by the ioctl.
     *
     * Note: the gateways parameter is unused if :
     * - returnedFirmwareStatus != ADSP_IPC_SUCCESS
     *
     * @throw Device::Exception
     */
    void addGetGatewaysCommand(dsp_fw::IxcStatus returnedFirmwareStatus, uint32_t gatewayCount,
                               const std::vector<dsp_fw::GatewayProps> &gateways);

    /**
     * Add a get fw config command.
     *
     * @param[in] returnedFirmwareStatus the returned firmware status
     * @param[in] fwConfigTlvList the fw config returned by the ioctl, which is a TLV list.
     *
     * Note: the fwConfigTlvList parameter is unused if :
     * - returnedFirmwareStatus != ADSP_IPC_SUCCESS
     *
     * @throw Device::Exception
     */
    void addGetFwConfigCommand(dsp_fw::IxcStatus returnedFirmwareStatus,
                               const util::Buffer &fwConfigTlvList);

    /** Add a get module instance properties command.
    *
    * @param[in] returnedFirmwareStatus the returned firmware status
    * @param[in] moduleId the id of the requested module type
    * @param[in] instanceId the id of the requested module instance
    * @param[in] props the module instance properties returned by the debugfs.
    *
    * Note: the props parameter is unused if :
    * - returnedFirmwareStatus != ADSP_IPC_SUCCESS
    *
    * @throw Device::Exception
    */
    void addGetModuleInstancePropsCommand(dsp_fw::IxcStatus returnedFirmwareStatus,
                                          uint16_t moduleId, uint16_t instanceId,
                                          const dsp_fw::ModuleInstanceProps &props);

    /** Add a get module parameter command.
    *
    * @param[in] returnedFirmwareStatus the returned firmware status
    * @param[in] moduleId the id of the requested module type
    * @param[in] instanceId the id of the requested module instance
    * @param[in] parameterId the id of the requested module parameter
    * @param[in] parameterPayloadSize the parameter payload size provided to the debugfs.
    * @param[in] parameterPayload the parameter payload provided to the debugfs.
    *
    * Note: the parameterPayload parameter is unused if :
    * - returnedFirmwareStatus != ADSP_IPC_SUCCESS
    *
    * @throw Device::Exception
    */
    void addGetModuleParameterCommand(dsp_fw::IxcStatus returnedFirmwareStatus, uint16_t moduleId,
                                      uint16_t instanceId, dsp_fw::ParameterId parameterTypeId,
                                      size_t parameterPayloadSize,
                                      const util::Buffer &parameterPayload);

    /** Add a get module parameter command.
    *
    * @param[in] returnedFirmwareStatus the returned firmware status
    * @param[in] moduleId the id of the requested module type
    * @param[in] instanceId the id of the requested module instance
    * @param[in] parameterId the id of the requested module parameter
    * @param[in] parameterPayload the parameter payload provided to the debugfs.
    *
    * Note: the parameterPayload parameter is unused if :
    * - returnedFirmwareStatus != ADSP_IPC_SUCCESS
    *
    * @throw Device::Exception
    */
    void addGetModuleParameterCommand(dsp_fw::IxcStatus returnedFirmwareStatus, uint16_t moduleId,
                                      uint16_t instanceId, dsp_fw::ParameterId parameterTypeId,
                                      const util::Buffer &parameterPayload);

    /** Add a set module parameter command.
    *
    * @param[in] returnedFirmwareStatus the returned firmware status
    * @param[in] moduleId the id of the requested module type
    * @param[in] instanceId the id of the requested module instance
    * @param[in] parameterId the id of the requested module parameter
    * @param[in] parameterPayload the parameter payload provided to the debugfs.
    *
    * @throw Device::Exception
    */
    void addSetModuleParameterCommand(dsp_fw::IxcStatus returnedFirmwareStatus, uint16_t moduleId,
                                      uint16_t instanceId, dsp_fw::ParameterId parameterTypeId,
                                      const util::Buffer &parameterPayload);

    /** Add a set module parameter command.
    *
    * @param[in] returnedFirmwareStatus the returned firmware status
    * @param[in] moduleId the id of the requested module type
    * @param[in] instanceId the id of the requested module instance
    * @param[in] parameterIdData the id+data of the requested module parameter to set
    *
    * @throw Device::Exception
    */
    void addSetModuleParameterCommand(dsp_fw::IxcStatus returnedFirmwareStatus, uint16_t moduleId,
                                      uint16_t instanceId, dsp_fw::ParameterId parameterIdData);

    /** Add a get module entries command.
     *
     * @param[in] returnedFirmwareStatus the returned firmware status
     * @param[in] moduleCount the exact module count
     * @param[in] returnedEntries the module entries returned by the ioctl.
     *
     * Note: the returnedEntries parameter is unused if :
     * - returnedFirmwareStatus != ADSP_IPC_SUCCESS
     *
     * @throw Device::Exception
     */
    void addGetModuleEntriesCommand(dsp_fw::IxcStatus returnedFirmwareStatus, uint32_t moduleCount,
                                    const std::vector<dsp_fw::ModuleEntry> &returnedEntries);

    /** Add a get global perf data command.
     *
     * @param[in] returnedFirmwareStatus the returned firmware status
     * @param[in] maxItemCount the number of items the firmware can return
     * @param[in] perfItems the list of perf data returned by the ioctl.
     *
     * Note: the perfItems parameter is unused if :
     * - returnedFirmwareStatus != ADSP_IPC_SUCCESS
     *
     * @throw Device::Exception
     */
    void addGetGlobalPerfDataCommand(dsp_fw::IxcStatus returnedFirmwareStatus,
                                     uint32_t maxItemCount,
                                     const std::vector<dsp_fw::PerfDataItem> &perfItems);

    /** Add a get global memory state command
     *
     * @param[in] returnedFirmwareStatus the returned firmware status
     * @param[in] memoryTlvList the memory state returned by the ioctl, which is a TLV list.
     *
     * Note: the memoryTlvList parameter is unused if :
     * - returnedFirmwareStatus != ADSP_IPC_SUCCESS
     *
     * @throw Device::Exception
     */
    void addGetGlobalMemoryStateCommand(dsp_fw::IxcStatus returnedFirmwareStatus,
                                        const util::Buffer &memoryTlvList);

    /** Add a set core power entries command.
     *
     * @param[in] controlSuccess true if the command shall be successful, false otherwise
     * @param[in] coreId core expected to be concerned by the command
     * @param[in] allowedToSleep true if the core is allowed to sleep, false otherwise.
     *
     * @throw Device::Exception
     */
    void addSetCorePowerCommand(bool controlSuccess, unsigned int coreId, bool allowedToSleep);

    void addGetPerfState(Perf::State state);
    void addSetPerfState(Perf::State state);

private:
    MockedDeviceCommands(const MockedDeviceCommands &) = delete;
    MockedDeviceCommands &operator=(const MockedDeviceCommands &) = delete;

    void addTlvParameterCommand(dsp_fw::IxcStatus returnedFirmwareStatus,
                                const util::Buffer &tlvList, dsp_fw::BaseFwParams parameterId);

    MockedDevice &mDevice;
};
}
}
}

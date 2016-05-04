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

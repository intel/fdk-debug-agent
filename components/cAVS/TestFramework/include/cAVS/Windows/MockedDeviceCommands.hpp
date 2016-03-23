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

#include "cAVS/DspFw/ModuleType.hpp"
#include "cAVS/DspFw/ModuleInstance.hpp"
#include "cAVS/DspFw/FwConfig.hpp"
#include "cAVS/DspFw/HwConfig.hpp"
#include "cAVS/DspFw/Pipeline.hpp"
#include "cAVS/DspFw/Gateway.hpp"
#include "cAVS/DspFw/Scheduler.hpp"
#include "cAVS/DspFw/Infrastructure.hpp"
#include "cAVS/Windows/DriverTypes.hpp"
#include "cAVS/Windows/MockedDevice.hpp"
#include "cAVS/Windows/IoCtlDescription.hpp"
#include "Util/Buffer.hpp"
#include <vector>

namespace debug_agent
{
namespace cavs
{
namespace windows
{

/** This class helps to feed a mocked device test vector by adding ioctl commands */
class MockedDeviceCommands final
{
public:
    MockedDeviceCommands(MockedDevice &device) : mDevice(device) {}
    /** Add a get module entries command.
     *
     * @param[in] ioctlSuccess the returned OS status (when calling DeviceIoControl)
     * @param[in] returnedDriverStatus the returned driver status
     * @param[in] returnedFirmwareStatus the returned firmware status
     * @param[in] moduleCount the exact module count
     * @param[in] returnedEntries the module entries returned by the ioctl.
     *
     * Note: the returnedEntries parameter is unused if :
     * - ioctlSuccess is false or
     * - NT_SUCCESS(returnedDriverStatus) returns false or
     * - returnedFirmwareStatus != ADSP_IPC_SUCCESS
     *
     * @throw Device::Exception
     */
    void addGetModuleEntriesCommand(bool ioctlSuccess, NTSTATUS returnedDriverStatus,
                                    dsp_fw::IxcStatus returnedFirmwareStatus, uint32_t moduleCount,
                                    const std::vector<dsp_fw::ModuleEntry> &returnedEntries);

    /**
     * Add a get fw config command.
     *
     * @param[in] ioctlSuccess the returned OS status (when calling DeviceIoControl)
     * @param[in] returnedDriverStatus the returned driver status
     * @param[in] returnedFirmwareStatus the returned firmware status
     * @param[in] fwConfigTlvList the fw config returned by the ioctl, which is a TLV list.
     *
     * Note: the fwConfigTlvList parameter is unused if :
     * - ioctlSuccess is false or
     * - NT_SUCCESS(returnedDriverStatus) returns false or
     * - returnedFirmwareStatus != ADSP_IPC_SUCCESS
     *
     * @throw Device::Exception
     */
    void addGetFwConfigCommand(bool ioctlSuccess, NTSTATUS returnedDriverStatus,
                               dsp_fw::IxcStatus returnedFirmwareStatus,
                               const util::Buffer &fwConfigTlvList);

    /**
     * Add a get hw config command.
     *
     * @param[in] ioctlSuccess the returned OS status (when calling DeviceIoControl)
     * @param[in] returnedDriverStatus the returned driver status
     * @param[in] returnedFirmwareStatus the returned firmware status
     * @param[in] hwConfigTlvList the hw config returned by the ioctl, which is a TLV list.
     *
     * Note: the hwConfigTlvList parameter is unused if :
     * - ioctlSuccess is false or
     * - NT_SUCCESS(returnedDriverStatus) returns false or
     * - returnedFirmwareStatus != ADSP_IPC_SUCCESS
     *
     * @throw Device::Exception
     */
    void addGetHwConfigCommand(bool ioctlSuccess, NTSTATUS returnedDriverStatus,
                               dsp_fw::IxcStatus returnedFirmwareStatus,
                               const util::Buffer &hwConfigTlvList);

    /** Add a get log parameters command
    *
    * @param[in] ioctlSuccess the returned OS status (when calling DeviceIoControl)
    * @param[in] returnedStatus the returned driver status
    * @param[in] returnedState the log parameters returned by the ioctl
    *
    * Note: returnedState is unused if
    * - ioctlSuccess is false or
    * - NT_SUCCESS(returnedStatus) returns false
    *
    * @throw Device::Exception
    */
    void addGetLogParametersCommand(bool ioctlSuccess, NTSTATUS returnedStatus,
                                    const driver::IoctlFwLogsState &returnedState);

    /** Add a set log parameters command.
     *
     * @param[in] ioctlSuccess the returned OS status (when calling DeviceIoControl)
     * @param[in] returnedStatus the returned driver status
     * @param[in] expectedState the expected log parameters passed as input buffer to the ioctl
     *
     * Note: the expectedState parameter is alwayse used,
     * even if NT_SUCCESS(returnedStatus) returns false or if ioctlSuccess is false
     *
     * @throw Device::Exception
     */
    void addSetLogParametersCommand(bool ioctlSuccess, NTSTATUS returnedStatus,
                                    const driver::IoctlFwLogsState &expectedState);

    /** Add a get probe state command
     *
     * @param[in] ioctlSuccess the returned OS status (when calling DeviceIoControl)
     * @param[in] returnedStatus the returned driver status
     * @param[in] returnedState the probe state returned by the ioctl
     *
     * Note: returnedState is unused if
     * - ioctlSuccess is false or
     * - NT_SUCCESS(returnedStatus) returns false
     *
     * @throw Device::Exception
     */
    void addGetProbeStateCommand(bool ioctlSuccess, NTSTATUS returnedStatus,
                                 driver::ProbeState returnedState);

    /** Add a set probe state command.
     *
     * @param[in] ioctlSuccess the returned OS status (when calling DeviceIoControl)
     * @param[in] returnedStatus the returned driver status
     * @param[in] expectedState the expected probe state passed as input buffer to the ioctl
     *
     * Note: the expectedState parameter is alwayse used,
     * even if NT_SUCCESS(returnedStatus) returns false or if ioctlSuccess is false
     *
     * @throw Device::Exception
     */
    void addSetProbeStateCommand(bool ioctlSuccess, NTSTATUS returnedStatus,
                                 driver::ProbeState expectedState);

    /** Add a get probe configuration command
     *
     * @param[in] ioctlSuccess the returned OS status (when calling DeviceIoControl)
     * @param[in] returnedStatus the returned driver status
     * @param[in] returnedConfiguration the probe configuration returned by the ioctl
     *
     * Note: returnedConfiguration is unused if
     * - ioctlSuccess is false or
     * - NT_SUCCESS(returnedStatus) returns false
     *
     * @throw Device::Exception
     */
    void addGetProbeConfigurationCommand(
        bool ioctlSuccess, NTSTATUS returnedStatus,
        const driver::ProbePointConfiguration &returnedConfiguration);

    /** Add a set probe configuration command.
     *
     * @param[in] ioctlSuccess the returned OS status (when calling DeviceIoControl)
     * @param[in] returnedStatus the returned driver status
     * @param[in] expectedConfiguration the expected probe configuration passed as
     *                                  input buffer to the ioctl
     *
     * Note: the expectedConfiguration parameter is alwayse used,
     * even if NT_SUCCESS(returnedStatus) returns false or if ioctlSuccess is false
     *
     * @throw Device::Exception
     */
    void addSetProbeConfigurationCommand(
        bool ioctlSuccess, NTSTATUS returnedStatus,
        const driver::ProbePointConfiguration &expectedConfiguration);

    /** Add a get ring buffers command
     *
     * @param[in] ioctlSuccess the returned OS status (when calling DeviceIoControl)
     * @param[in] returnedStatus the returned driver status
     * @param[in] returnedBuffers the buffers description returned by the ioctl
     *
     * @note: returnedBuffers is unused if
     * - ioctlSuccess is false or
     * - NT_SUCCESS(returnedStatus) returns false
     */
    void addGetRingBuffers(bool ioctlSuccess, NTSTATUS returnedStatus,
                           const driver::RingBuffersDescription &returnedBuffers);

    /** Add a get extraction ring buffer linear position command
     *
     * @param[in] ioctlSuccess the returned OS status (when calling DeviceIoControl)
     * @param[in] returnedStatus the returned driver status
     * @param[in] position the position reurned by the ioctl
     *
     * @note: position is unused if
     * - ioctlSuccess is false or
     * - NT_SUCCESS(returnedStatus) returns false
     */
    void addGetExtractionRingBufferLinearPosition(bool ioctlSuccess, NTSTATUS returnedStatus,
                                                  uint64_t position);

    /** Add a get pipeline list command.
     *
     * @param[in] ioctlSuccess the returned OS status (when calling DeviceIoControl)
     * @param[in] returnedDriverStatus the returned driver status
     * @param[in] returnedFirmwareStatus the returned firmware status
     * @param[in] maxPplCount the maximum pipeline count
     * @param[in] pipelineIds the pipeline id list returned by the ioctl.
     *
     * Note: the pipelineIds parameter is unused if :
     * - ioctlSuccess is false or
     * - NT_SUCCESS(returnedDriverStatus) returns false or
     * - returnedFirmwareStatus != ADSP_IPC_SUCCESS
     *
     * @throw Device::Exception
     */
    void addGetPipelineListCommand(bool ioctlSuccess, NTSTATUS returnedDriverStatus,
                                   dsp_fw::IxcStatus returnedFirmwareStatus, uint32_t maxPplCount,
                                   const std::vector<dsp_fw::PipeLineIdType> &pipelineIds);

    /** Add a get pipeline props command.
     *
     * @param[in] ioctlSuccess the returned OS status (when calling DeviceIoControl)
     * @param[in] returnedDriverStatus the returned driver status
     * @param[in] returnedFirmwareStatus the returned firmware status
     * @param[in] pipelineId the id of the requested pipeline
     * @param[in] pipelineProps the returned pipeline properties
     *
     * Note: the pipelineProps parameter is unused if :
     * - ioctlSuccess is false or
     * - NT_SUCCESS(returnedDriverStatus) returns false or
     * - returnedFirmwareStatus != ADSP_IPC_SUCCESS
     *
     * @throw Device::Exception
     */
    void addGetPipelinePropsCommand(bool ioctlSuccess, NTSTATUS returnedDriverStatus,
                                    dsp_fw::IxcStatus returnedFirmwareStatus,
                                    dsp_fw::PipeLineIdType pipelineId,
                                    const dsp_fw::PplProps &pipelineProps);

    /** Add a get schedulers info command.
     *
     * @param[in] ioctlSuccess the returned OS status (when calling DeviceIoControl)
     * @param[in] returnedDriverStatus the returned driver status
     * @param[in] returnedFirmwareStatus the returned firmware status
     * @param[in] coreId the id of the requested core
     * @param[in] info the returned schedulers info
     *
     * Note: the info parameter is unused if :
     * - ioctlSuccess is false or
     * - NT_SUCCESS(returnedDriverStatus) returns false or
     * - returnedFirmwareStatus != ADSP_IPC_SUCCESS
     *
     * @throw Device::Exception
     */
    void addGetSchedulersInfoCommand(bool ioctlSuccess, NTSTATUS returnedDriverStatus,
                                     dsp_fw::IxcStatus returnedFirmwareStatus,
                                     dsp_fw::CoreId coreId, const dsp_fw::SchedulersInfo &info);

    /** Add a get gateways command.
     *
     * @param[in] ioctlSuccess the returned OS status (when calling DeviceIoControl)
     * @param[in] returnedDriverStatus the returned driver status
     * @param[in] returnedFirmwareStatus the returned firmware status
     * @param[in] gatewayCount the gateway count
     * @param[in] gateways the gateway list returned by the ioctl.
     *
     * Note: the gateways parameter is unused if :
     * - ioctlSuccess is false or
     * - NT_SUCCESS(returnedDriverStatus) returns false or
     * - returnedFirmwareStatus != ADSP_IPC_SUCCESS
     *
     * @throw Device::Exception
     */
    void addGetGatewaysCommand(bool ioctlSuccess, NTSTATUS returnedDriverStatus,
                               dsp_fw::IxcStatus returnedFirmwareStatus, uint32_t gatewayCount,
                               const std::vector<dsp_fw::GatewayProps> &gateways);

    /** Add a get module instance properties command.
    *
    * @param[in] ioctlSuccess the returned OS status (when calling DeviceIoControl)
    * @param[in] returnedDriverStatus the returned driver status
    * @param[in] returnedFirmwareStatus the returned firmware status
    * @param[in] moduleId the id of the requested module type
    * @param[in] instanceId the id of the requested module instance
    * @param[in] props the module instance properties returned by the ioctl.
    *
    * Note: the props parameter is unused if :
    * - ioctlSuccess is false or
    * - NT_SUCCESS(returnedDriverStatus) returns false or
    * - returnedFirmwareStatus != ADSP_IPC_SUCCESS
    *
    * @throw Device::Exception
    */
    void addGetModuleInstancePropsCommand(bool ioctlSuccess, NTSTATUS returnedDriverStatus,
                                          dsp_fw::IxcStatus returnedFirmwareStatus,
                                          uint16_t moduleId, uint16_t instanceId,
                                          const dsp_fw::ModuleInstanceProps &props);

    /** Add a set module parameter command.
    *
    * @param[in] ioctlSuccess the returned OS status (when calling DeviceIoControl)
    * @param[in] returnedDriverStatus the returned driver status
    * @param[in] returnedFirmwareStatus the returned firmware status
    * @param[in] moduleId the id of the requested module type
    * @param[in] instanceId the id of the requested module instance
    * @param[in] parameterId the id of the requested module parameter
    * @param[in] parameterPayload the parameter payload provided to the ioctl.
    *
    * @throw Device::Exception
    */
    void addSetModuleParameterCommand(bool ioctlSuccess, NTSTATUS returnedDriverStatus,
                                      dsp_fw::IxcStatus returnedFirmwareStatus, uint16_t moduleId,
                                      uint16_t instanceId, dsp_fw::ParameterId parameterId,
                                      const util::Buffer &parameterPayload);

    /** Add a get module parameter command.
    *
    * @param[in] ioctlSuccess the returned OS status (when calling DeviceIoControl)
    * @param[in] returnedDriverStatus the returned driver status
    * @param[in] returnedFirmwareStatus the returned firmware status
    * @param[in] moduleId the id of the requested module type
    * @param[in] instanceId the id of the requested module instance
    * @param[in] parameterId the id of the requested module parameter
    * @param[in] parameterPayload the parameter payload provided to the ioctl.
    *
    * Note: the parameterPayload parameter is unused if :
    * - ioctlSuccess is false or
    * - NT_SUCCESS(returnedDriverStatus) returns false or
    * - returnedFirmwareStatus != ADSP_IPC_SUCCESS
    *
    * @throw Device::Exception
    */
    void addGetModuleParameterCommand(bool ioctlSuccess, NTSTATUS returnedDriverStatus,
                                      dsp_fw::IxcStatus returnedFirmwareStatus, uint16_t moduleId,
                                      uint16_t instanceId, dsp_fw::ParameterId parameterId,
                                      const util::Buffer &parameterPayload);

private:
    MockedDeviceCommands(const MockedDeviceCommands &) = delete;
    MockedDeviceCommands &operator=(const MockedDeviceCommands &) = delete;

    enum class Command
    {
        Get,
        Set
    };

    void addTlvParameterCommand(bool ioctlSuccess, NTSTATUS returnedDriverStatus,
                                dsp_fw::IxcStatus returnedFirmwareStatus,
                                const util::Buffer &tlvList, dsp_fw::BaseFwParams parameterId);

    /** Common method to add "module access param" ioctl command */
    void addModuleParameterCommand(Command command, uint16_t moduleId, uint16_t instanceId,
                                   dsp_fw::ParameterId parameterTypeId,
                                   const util::Buffer &expectedParameterContent,
                                   const util::Buffer &returnedParameterContent, bool ioctlSuccess,
                                   NTSTATUS returnedDriverStatus,
                                   dsp_fw::IxcStatus returnedFirmwareStatus);

    /** Template method that aims to add a "get module access param" ioctl command using
     * the supplied firmware parameter
     *
     * @FirmwareParameterType The type of the firwmare parameter that will be returned
     */
    template <typename FirmwareParameterType>
    void addGetModuleParameterCommand(uint16_t moduleId, uint16_t instanceId,
                                      dsp_fw::ParameterId parameterTypeId,
                                      std::size_t expectedOutputBufferSize,
                                      const FirmwareParameterType &parameter, bool ioctlSuccess,
                                      NTSTATUS returnedDriverStatus,
                                      dsp_fw::IxcStatus returnedFirmwareStatus);

    template <driver::IOCTL_FEATURE feature, uint32_t parameter, typename DriverStructure>
    void addTinyGetCommand(const DriverStructure &returnedDriverStruct, bool ioctlSuccess,
                           NTSTATUS returnedDriverStatus);

    template <driver::IOCTL_FEATURE feature, uint32_t parameter, typename DriverStructure>
    void addTinySetCommand(const DriverStructure &inputDriverStruct, bool ioctlSuccess,
                           NTSTATUS returnedDriverStatus);

    /* Performs a TinyGet/Set command with a supplied ioctl description passed as template parameter
     */
    template <class IoCtlDescription>
    void addTinyCommand(typename const IoCtlDescription::Data &inputDriverStr,
                        typename const IoCtlDescription::Data &outputDriverStr, bool ioctlSuccess,
                        NTSTATUS returnedDriverStatus);

    MockedDevice &mDevice;
};
}
}
}

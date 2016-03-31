/*
 ********************************************************************************
 *                              INTEL CONFIDENTIAL
 *   Copyright(C) 2015-2016 Intel Corporation. All Rights Reserved.
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

/* Windows types must be included before driver headers */
#include "cAVS/Windows/WindowsTypes.hpp"

/* Including driver header in a private namespace */
namespace private_driver
{
#include <IIntcPrivateIOCTL.h>
}

#include "cAVS/DspFw/Common.hpp"
#include "Util/ByteStreamReader.hpp"
#include "Util/ByteStreamWriter.hpp"
#include "Util/StructureChangeTracking.hpp"
#include <inttypes.h>
#include <utility>
#include <stdexcept>

namespace debug_agent
{
namespace cavs
{
namespace windows
{
namespace driver
{

struct Exception : std::runtime_error
{
    using std::runtime_error::runtime_error;
};

/** IO control types supported by the cAVS driver
 */
enum class IoCtlType
{
    GetApiVersion = IOCTL_CMD_APP_TO_AUDIODSP_GET_APIVERSION,
    IsSupported = IOCTL_CMD_APP_TO_AUDIODSP_ISSUPPORTED,
    TinySet = IOCTL_CMD_APP_TO_AUDIODSP_TINY_SET,
    BigSet = IOCTL_CMD_APP_TO_AUDIODSP_BIG_SET,
    TinyGet = IOCTL_CMD_APP_TO_AUDIODSP_TINY_GET,
    BigGet = IOCTL_CMD_APP_TO_AUDIODSP_BIG_GET,
};

std::string to_string(IoCtlType type);

/** By convention the parameter id of the "MODULE PARAMETER ACCESS" feature is 0 */
static const uint32_t moduleParameterAccessCommandParameterId = 0;

/** By convention the parameter id of the "LOG_PARAMETERS" feature is 0 */
static const uint32_t logParametersCommandparameterId = 0;

/* Importing IOCTL_FEATURE enum */
using IOCTL_FEATURE = private_driver::IOCTL_FEATURE;

/* Importing IOCTL_LOG_STATE enum */
using IOCTL_LOG_STATE =
    private_driver::IntcIoctlFeatures::FwLogsFeature::LogStatusParam::StartedValueType;

/* Importing FW_LOG_LEVEL enum */
using FW_LOG_LEVEL =
    private_driver::IntcIoctlFeatures::FwLogsFeature::LogStatusParam::LevelValueType;

/* Importing FW_LOG_OUTPUT enum */
using FW_LOG_OUTPUT =
    private_driver::IntcIoctlFeatures::FwLogsFeature::LogStatusParam::OutputValueType;

/* Intc_App_Cmd_Header */

CHECK_SIZE(private_driver::Intc_App_Cmd_Header, 20);
CHECK_MEMBER(private_driver::Intc_App_Cmd_Header, FeatureID, 0, ULONG);
CHECK_MEMBER(private_driver::Intc_App_Cmd_Header, ParameterID, 4, ULONG);
CHECK_MEMBER(private_driver::Intc_App_Cmd_Header, Reserved, 8, ULONG);
CHECK_MEMBER(private_driver::Intc_App_Cmd_Header, SetAsDefault, 12, BOOL);
CHECK_MEMBER(private_driver::Intc_App_Cmd_Header, DataSize, 16, ULONG);

struct Intc_App_Cmd_Header
{
    ULONG FeatureID;
    ULONG ParameterID;
    ULONG Reserved;
    BOOL SetAsDefault;
    ULONG DataSize;

    Intc_App_Cmd_Header(ULONG featureID, ULONG parameterID, ULONG dataSize)
        : FeatureID(featureID), ParameterID(parameterID), Reserved(0), SetAsDefault(FALSE),
          DataSize(dataSize)
    {
    }

    bool operator==(const Intc_App_Cmd_Header &other)
    {
        return FeatureID == other.FeatureID && ParameterID == other.ParameterID &&
               Reserved == other.Reserved && SetAsDefault == other.SetAsDefault &&
               DataSize == other.DataSize;
    }

    void fromStream(util::ByteStreamReader &reader)
    {
        reader.read(FeatureID);
        reader.read(ParameterID);
        reader.read(Reserved);
        reader.read(SetAsDefault);
        reader.read(DataSize);
    }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.write(FeatureID);
        writer.write(ParameterID);
        writer.write(Reserved);
        writer.write(SetAsDefault);
        writer.write(DataSize);
    }
};

/* Intc_App_Cmd_Body */

CHECK_SIZE(private_driver::Intc_App_Cmd_Body, 12);
CHECK_MEMBER(private_driver::Intc_App_Cmd_Body, Status, 0, NTSTATUS);
CHECK_MEMBER(private_driver::Intc_App_Cmd_Body, Data, 4,
             private_driver::Intc_App_Cmd_Body::_BodyData);

struct Intc_App_Cmd_Body
{
    NTSTATUS Status;

    Intc_App_Cmd_Body() : Status(0xFFFFFFFF) {}

    bool operator==(const Intc_App_Cmd_Body &other) { return Status == other.Status; }

    void fromStream(util::ByteStreamReader &reader) { reader.read(Status); }

    void toStream(util::ByteStreamWriter &writer) const { writer.write(Status); }
};

/* IoctlFwModuleParam */
using private_IoctlFwModuleAccessParam =
    private_driver::IntcIoctlFeatures::FwModuleParamFeature::FwModuleParamAccessParam::BufferStruct;
CHECK_SIZE(private_IoctlFwModuleAccessParam, 17); // last byte is the dynamic payload artefact
CHECK_MEMBER(private_IoctlFwModuleAccessParam, fw_status, 0, UINT32);
CHECK_MEMBER(private_IoctlFwModuleAccessParam, instance_id, 4, UINT16);
CHECK_MEMBER(private_IoctlFwModuleAccessParam, module_id, 6, UINT16);
CHECK_MEMBER(private_IoctlFwModuleAccessParam, module_parameter_id, 8, UINT32);
CHECK_MEMBER(private_IoctlFwModuleAccessParam, module_parameter_data_size, 12, UINT32);
CHECK_MEMBER(private_IoctlFwModuleAccessParam, module_parameter_data, 16, UINT8[1]);

struct IoctlFwModuleParam
{
    ULONG fw_status;
    USHORT instance_id;
    USHORT module_id;
    dsp_fw::ParameterId module_parameter_id;
    ULONG module_parameter_data_size;

    IoctlFwModuleParam(USHORT moduleId, USHORT instanceId, dsp_fw::ParameterId moduleParameterId,
                       ULONG moduleParameterDataSize)
        : fw_status(0xFFFFFFFF), instance_id(instanceId), module_id(moduleId),
          module_parameter_id(moduleParameterId),
          module_parameter_data_size(moduleParameterDataSize)
    {
    }

    bool operator==(const IoctlFwModuleParam &other)
    {
        return fw_status == other.fw_status && instance_id == other.instance_id &&
               module_id == other.module_id && module_parameter_id == other.module_parameter_id &&
               module_parameter_data_size == other.module_parameter_data_size;
    }

    void fromStream(util::ByteStreamReader &reader)
    {
        reader.read(fw_status);
        reader.read(instance_id);
        reader.read(module_id);
        reader.read(module_parameter_id);
        reader.read(module_parameter_data_size);
    }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.write(fw_status);
        writer.write(instance_id);
        writer.write(module_id);
        writer.write(module_parameter_id);
        writer.write(module_parameter_data_size);
    }
};

/* IoctlFwLogsState */

using private_IoctlFwLogsState =
    private_driver::IntcIoctlFeatures::FwLogsFeature::LogStatusParam::BufferStruct;
CHECK_SIZE(private_IoctlFwLogsState, 12);
CHECK_MEMBER(private_IoctlFwLogsState, started, 0, IOCTL_LOG_STATE);
CHECK_MEMBER(private_IoctlFwLogsState, level, 4, FW_LOG_LEVEL);
CHECK_MEMBER(private_IoctlFwLogsState, output, 8, FW_LOG_OUTPUT);

struct IoctlFwLogsState
{
    IOCTL_LOG_STATE started;
    FW_LOG_LEVEL level;
    FW_LOG_OUTPUT output;

    bool operator==(const IoctlFwLogsState &other)
    {
        return started == other.started && level == other.level && output == other.output;
    }

    void fromStream(util::ByteStreamReader &reader)
    {
        reader.read(started);
        reader.read(level);
        reader.read(output);
    }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.write(started);
        writer.write(level);
        writer.write(output);
    }
};

/** State of the probing service */

/** Importing Probe feature param enum */
using ProbeFeatureParameter = private_driver::IntcIoctlFeatures::FwProbeFeature::Params;

/** Importing Probe state enum */
using ProbeState =
    private_driver::IntcIoctlFeatures::FwProbeFeature::FeatureStateParam::ProbeFeatureState;

/** Importing probe count */
static constexpr uint32_t maxProbes =
    private_driver::IntcIoctlFeatures::FwProbeFeature::MAX_INJECTION_BUFFERS;

/** Importing probe type enum */
using ProbeType =
    private_driver::IntcIoctlFeatures::FwProbeFeature::PointConfigurationParam::ProbePointType;

/** Importing probe purpose enum */
using ProbePurpose = private_driver::IntcIoctlFeatures::FwProbeFeature::PointConfigurationParam::
    ProbePointConnectionPurpose;

/** ProbePointId */
using private_ProbePointId =
    private_driver::IntcIoctlFeatures::FwProbeFeature::PointConfigurationParam::ProbePointId;
CHECK_SIZE(private_ProbePointId, 4);
CHECK_MEMBER(private_ProbePointId, full, 0, UINT32);

union ProbePointId
{
    static constexpr int moduleIdSize{16};
    static constexpr int instanceIdSize{8};
    static constexpr int typeSize{2};
    static constexpr int indexSize{6};

    struct
    {
        uint32_t moduleId : moduleIdSize;
        uint32_t instanceId : instanceIdSize;
        uint32_t type : typeSize;
        uint32_t index : indexSize;
    } fields;
    uint32_t full;

    bool operator==(const ProbePointId &other) { return full == other.full; }

    void fromStream(util::ByteStreamReader &reader) { reader.read(full); }

    void toStream(util::ByteStreamWriter &writer) const { writer.write(full); }
};

/** EventHandleType */
using private_EventHandleType =
    private_driver::IntcIoctlFeatures::FwProbeFeature::PointConfigurationParam::EventHandleType;
CHECK_SIZE(private_EventHandleType, 8);

/** ProbePointConnection */
using private_EventHandle = private_driver::IntcIoctlFeatures::FwProbeFeature::
    PointConfigurationParam::ProbePointConnection;

using private_ProbePointConnection = private_driver::IntcIoctlFeatures::FwProbeFeature::
    PointConfigurationParam::ProbePointConnection;
CHECK_SIZE(private_ProbePointConnection, 20);
CHECK_MEMBER(private_ProbePointConnection, enabled, 0, BOOL);
CHECK_MEMBER(private_ProbePointConnection, probePointId, 4, private_ProbePointId);
CHECK_MEMBER(private_ProbePointConnection, purpose, 8, ProbePurpose);
CHECK_MEMBER(private_ProbePointConnection, injectionBufferCompletionEventHandle, 12,
             private_EventHandleType);

struct ProbePointConnection
{
    bool enabled;
    ProbePointId probePointId;
    ProbePurpose purpose;
    HANDLE injectionBufferCompletionEventHandle;

    ProbePointConnection() = default;
    ProbePointConnection(bool enabled, ProbePointId probePointId, ProbePurpose purpose,
                         HANDLE handle)
        : enabled(enabled), probePointId(probePointId), purpose(purpose),
          injectionBufferCompletionEventHandle(handle)
    {
    }

    void fromStream(util::ByteStreamReader &reader)
    {
        reader.read(enabled);
        reader.read(probePointId);
        reader.read(purpose);
        reader.read(injectionBufferCompletionEventHandle);
    }
    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.write(enabled);
        writer.write(probePointId);
        writer.write(purpose);
        writer.write(injectionBufferCompletionEventHandle);
    }
};

/** ProbePointConnection */
using private_ProbePointConfiguration =
    private_driver::IntcIoctlFeatures::FwProbeFeature::PointConfigurationParam::BufferStruct;
CHECK_SIZE(private_ProbePointConfiguration, 168);
CHECK_MEMBER(private_ProbePointConfiguration, extractionBufferCompletionEventHandle, 0,
             private_EventHandleType);
CHECK_MEMBER(private_ProbePointConfiguration, probePointConnections, 8,
             private_ProbePointConnection[maxProbes]);

struct ProbePointConfiguration
{
    HANDLE extractionBufferCompletionEventHandle;
    ProbePointConnection probePointConnection[maxProbes];

    void fromStream(util::ByteStreamReader &reader)
    {
        reader.read(extractionBufferCompletionEventHandle);
        reader.read(probePointConnection);
    }
    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.write(extractionBufferCompletionEventHandle);
        writer.write(probePointConnection);
    }
};

/** UserModeBufferPointerType */
using private_UserModeBufferPointerType = private_driver::IntcIoctlFeatures::FwProbeFeature::
    BuffersDescriptionParam::UserModeBufferPointerType;
CHECK_SIZE(private_UserModeBufferPointerType, 8);

/** RingBufferDescription */
using private_RingBufferDescription = private_driver::IntcIoctlFeatures::FwProbeFeature::
    BuffersDescriptionParam::RingBufferDescription;
CHECK_SIZE(private_RingBufferDescription, 16);
CHECK_MEMBER(private_RingBufferDescription, pStartAddress, 0, private_UserModeBufferPointerType);
CHECK_MEMBER(private_RingBufferDescription, size, 8, size_t);

struct RingBufferDescription
{
    volatile uint8_t *startAdress;
    size_t size;

    void fromStream(util::ByteStreamReader &reader)
    {
        reader.read(startAdress);
        reader.read(size);
    }
    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.write(startAdress);
        writer.write(size);
    }
};

/** RingBuffersDescription */
using private_RingBuffersDescription =
    private_driver::IntcIoctlFeatures::FwProbeFeature::BuffersDescriptionParam::BufferStruct;
CHECK_SIZE(private_RingBuffersDescription, 16 + 16 * 8);
CHECK_MEMBER(private_RingBuffersDescription, extractionRBDescription, 0,
             private_RingBufferDescription);
CHECK_MEMBER(private_RingBuffersDescription, injectionRBDescriptions, 16,
             private_RingBufferDescription[maxProbes]);

struct RingBuffersDescription
{
    RingBufferDescription extractionRBDescription;
    std::array<RingBufferDescription, 8> injectionRBDescriptions;

    void fromStream(util::ByteStreamReader &reader)
    {
        reader.read(extractionRBDescription);
        reader.read(injectionRBDescriptions);
    }
    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.write(extractionRBDescription);
        writer.write(injectionRBDescriptions);
    }
};
}
}
}
}

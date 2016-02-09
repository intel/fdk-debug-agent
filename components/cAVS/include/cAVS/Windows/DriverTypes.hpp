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
using IOCTL_LOG_STATE = private_driver::IOCTL_LOG_STATE;

/* Importing FW_LOG_LEVEL enum */
using FW_LOG_LEVEL = private_driver::FW_LOG_LEVEL;

/* Importing FW_LOG_OUTPUT enum */
using FW_LOG_OUTPUT = private_driver::FW_LOG_OUTPUT;

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

CHECK_SIZE(private_driver::IoctlFwModuleParam, 20);
CHECK_MEMBER(private_driver::IoctlFwModuleParam, fw_status, 0, ULONG);
CHECK_MEMBER(private_driver::IoctlFwModuleParam, instance_id, 4, USHORT);
CHECK_MEMBER(private_driver::IoctlFwModuleParam, module_id, 6, USHORT);
CHECK_MEMBER(private_driver::IoctlFwModuleParam, module_parameter_id, 8, ULONG);
CHECK_MEMBER(private_driver::IoctlFwModuleParam, module_parameter_data_size, 12, ULONG);
CHECK_MEMBER(private_driver::IoctlFwModuleParam, module_parameter_data, 16, UCHAR[1]);

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

CHECK_SIZE(private_driver::IoctlFwLogsState, 12);
CHECK_MEMBER(private_driver::IoctlFwLogsState, started, 0, ULONG);
CHECK_MEMBER(private_driver::IoctlFwLogsState, level, 4, ULONG);
CHECK_MEMBER(private_driver::IoctlFwLogsState, output, 8, ULONG);

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
enum class ProbeState : uint32_t
{
    Idle,
    Owned,     /// probe service is owned (required because it's a monoclient service)
    Allocated, /// Buffers are allocated
    Active     /// Probing is running
};

// TODO: import from driver headers
static constexpr uint32_t maxProbes{8};

// TODO: import from driver headers
enum class ProbeType : uint32_t
{
    Input = 0,
    Output = 1,
    Internal = 2
};

// TODO: import from driver headers
enum class ProbePurpose : uint32_t
{
    Inject = 0,
    Extract = 1,
    InjectReextract = 2
};

// TODO: import from driver headers
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

// TODO: import from driver headers
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

// TODO: import from driver headers
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
}
}
}
}

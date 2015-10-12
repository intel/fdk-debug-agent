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

#include "cAVS/Windows/WindowsTypes.hpp"
#include "Util/ByteStreamReader.hpp"
#include "Util/ByteStreamWriter.hpp"
#include <inttypes.h>
#include <utility>

namespace debug_agent
{
namespace cavs
{
namespace windows
{
namespace driver
{

#define IOCTL_CMD_APP_TO_AUDIODSP_TINY_SET \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x97B, METHOD_BUFFERED, (FILE_READ_ACCESS | FILE_WRITE_ACCESS))

#define IOCTL_CMD_APP_TO_AUDIODSP_BIG_SET \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x97C, METHOD_OUT_DIRECT, (FILE_READ_ACCESS | FILE_WRITE_ACCESS))

#define IOCTL_CMD_APP_TO_AUDIODSP_TINY_GET \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x97D, METHOD_BUFFERED, (FILE_READ_ACCESS | FILE_WRITE_ACCESS))

#define IOCTL_CMD_APP_TO_AUDIODSP_BIG_GET \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x97E, METHOD_OUT_DIRECT, (FILE_READ_ACCESS | FILE_WRITE_ACCESS))

/** By convention the base firmware module id is 0 */
static const USHORT baseFirwareModuleId = 0;

/** By convention the base firmware instance id is 0 */
static const USHORT baseFirwareInstanceId = 0;

/** By convention the parameter id of the "MODULE PARAMETER ACCESS" feature is 0 */
static const uint32_t moduleParameterAccessCommandParameterId = 0;

/** By convention the parameter id of the "LOG_PARAMETERS" feature is 0 */
static const uint32_t logParametersCommandparameterId = 0;

enum class IOCTL_FEATURE
{
    FEATURE_WOV = 0x10000,
    FEATURE_AEC = 0x20000,
    FEATURE_MICSELECTOR = 0x30000,
    FEATURE_AWARENESS = 0x100000,
    FEATURE_FW_LOGS = 0x200000,
    FEATURE_PROBE_CAPTURE = 0x210000,
    FEATURE_PROBE_INJECT = 0x220000,
    FEATURE_TOPOLOGY_NOTIFICATION = 0x230000,
    FEATURE_FW_MODULE_PARAM = 0x240000,
    FEATURE_FW_WAKELOCK = 0x250000
};

enum class IOCTL_LOG_STATE : ULONG
{
    STOPPED = 0,
    STARTED = 1
};

enum class FW_LOG_LEVEL : ULONG
{
    LOG_CRITICAL = 2,
    LOG_HIGH = 3,
    LOG_MEDIUM = 4,
    LOG_LOW = 5,
    LOG_VERBOSE = 6
};

enum class FW_LOG_OUTPUT : ULONG
{
    OUTPUT_SRAM = 0,//Log are sent by Firmware to Log Buffer using dedicated DMA
    OUTPUT_PTI = 1 //Logs are sent by Firmware to PTI using Little Peak.
};

struct Intc_App_Cmd_Header
{
    ULONG FeatureID;
    ULONG ParameterID;
    ULONG Reserved;
    BOOL  SetAsDefault;
    ULONG DataSize;

    Intc_App_Cmd_Header(ULONG featureID, ULONG parameterID, ULONG dataSize) :
        FeatureID(featureID), ParameterID(parameterID), Reserved(0), SetAsDefault(FALSE),
        DataSize(dataSize) {}

    bool operator == (const Intc_App_Cmd_Header &other)
    {
        return FeatureID == other.FeatureID &&
            ParameterID == other.ParameterID &&
            Reserved == other.Reserved &&
            SetAsDefault == other.SetAsDefault &&
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

struct Intc_App_Cmd_Status
{
    NTSTATUS status;

    Intc_App_Cmd_Status() : status(0xFFFFFFFF) {}


    bool operator == (const Intc_App_Cmd_Status &other)
    {
        return status == other.status;
    }

    void fromStream(util::ByteStreamReader &reader)
    {
        reader.read(status);
    }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.write(status);
    }
};

template <class T>
struct Intc_App_TinyCmd
{
    Intc_App_Cmd_Header header;
    Intc_App_Cmd_Status status;
    T body;

    Intc_App_TinyCmd(ULONG featureID, ULONG parameterID, const T &body) :
        header(featureID, parameterID, sizeof(Intc_App_Cmd_Status) + sizeof(T)),
        status(),
        body(body)
    {
    }

    bool operator == (const Intc_App_TinyCmd &other)
    {
        return header == other.header &&
            status == other.status &&
            body == other.body;
    }

    void fromStream(util::ByteStreamReader &reader)
    {
        reader.read(header);
        reader.read(status);
        reader.read(body);
    }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.write(header);
        writer.write(status);
        writer.write(body);
    }
};

struct IoctlFwModuleParam
{
    ULONG  fw_status;
    USHORT instance_id;
    USHORT module_id;
    ULONG  module_parameter_id;
    ULONG  module_parameter_data_size;

    IoctlFwModuleParam(USHORT moduleId, USHORT instanceId, ULONG moduleParameterId,
        ULONG moduleParameterDataSize) : fw_status(0xFFFFFFFF),
        instance_id(instanceId), module_id(moduleId), module_parameter_id(moduleParameterId),
        module_parameter_data_size(moduleParameterDataSize)
    {
    }

    bool operator == (const IoctlFwModuleParam &other)
    {
        return fw_status == other.fw_status &&
            instance_id == other.instance_id &&
            module_id == other.module_id &&
            module_parameter_id == other.module_parameter_id &&
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

struct IoctlFwLogsState
{
    IOCTL_LOG_STATE started;
    FW_LOG_LEVEL level;
    FW_LOG_OUTPUT output;

    bool operator == (const IoctlFwLogsState &other)
    {
        return started == other.started &&
            level == other.level &&
            output == other.output;
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

}
}
}
}

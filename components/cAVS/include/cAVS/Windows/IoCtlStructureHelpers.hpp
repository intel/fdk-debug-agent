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

#include "cAVS/FirmwareTypes.hpp"
#include "cAVS/Windows/DriverTypes.hpp"
#include "cAVS/Windows/Buffer.hpp"

/** Returns the size of a struct member
 * This is needed because sizeof(MyStruct::member) doesn't work, and instance is required.
 * Here the trick is to use a null instance.
 */
#define MEMBER_SIZE(type, member) sizeof(static_cast<type*>(nullptr)->member)

namespace debug_agent
{
namespace cavs
{
namespace windows
{

/** By convention the base firmware module id is 0 */
static const uint32_t baseFirwareModuleId = 0;

/** By convention the base firmware instance id is 0 */
static const uint32_t baseFirwareInstanceId = 0;

/** This class helps to construct IOCTL_CMD_APP_TO_AUDIODSP_BIG_(GET|SET) output structure.
 * This structure has 3 imbrication levels:
 * Intc_Get_Parameter (driver structure) which contains
 *   -> IoctlFwModuleParam (driver structure) which contains
 *        -> FirmwareParameterType (firmware structure), for instance AdspProperties,
 *           ModulesInfo...
 *
 * See Intel SST Driver Private IOCTL Specification (FDK Support).docx
 */
template <typename FirmwareParameterType>
class BigCmdModuleAccessIoctlOutput
{
public:
    /**
     * @param[in] fwParam The firmware parameter type
     * @param[in] firmwareParameterSize the size of the firmware parameter.
     *                                   Must be >= sizeof(FirmwareParameterType)
     * @param[in] moduleId the id of the requested module type (optional, default is the base
     *                     firmware one).
     * @param[in] instanceId the id of the requested module instance (optional, default is the base
     *                     firmware one).
     */
    BigCmdModuleAccessIoctlOutput(uint32_t fwParam,
        std::size_t firmwareParameterSize,
        uint16_t moduleId = baseFirwareModuleId,
        uint16_t instanceId = baseFirwareInstanceId) :
        mBuffer(calculateSize(firmwareParameterSize)),
        mModuleParameterAccess(nullptr), mFirmwareParameter(nullptr)
    {
        /* Initializing pointer members */
        initPointers();

        /* Setting ModuleParameterAccess input properties */
        mModuleParameterAccess->module_id = moduleId;
        mModuleParameterAccess->instance_id = instanceId;
        mModuleParameterAccess->module_parameter_id = fwParam;
        mModuleParameterAccess->module_parameter_data_size = static_cast<uint32_t>(firmwareParameterSize);
    }

    /** Copy constructor */
    BigCmdModuleAccessIoctlOutput(const BigCmdModuleAccessIoctlOutput& other) :
        mBuffer(other.mBuffer)
    {
        initPointers();
    }

    /** @return Intc_App_Cmd_Body reference */
    driver::Intc_App_Cmd_Body &getCmdBody()
    {
        return mBuffer.getContent();
    }

    /** @return IoctlFwModuleParam reference */
    driver::IoctlFwModuleParam &getModuleParameterAccess()
    {
        return *mModuleParameterAccess;
    }

    /**
     * In case the parameter size is not static, the effective parameter size is required.
     * @param[out] parameterSize the effective size of the parameter
     * @return FirmwareParameterType reference
     */
    FirmwareParameterType &getFirmwareParameter(size_t &parameterSize)
    {
        parameterSize = mBuffer.getSize() - parameterOffset;
        return getFirmwareParameter();
    }

    /** @return FirmwareParameterType reference */
    FirmwareParameterType &getFirmwareParameter()
    {
        return *mFirmwareParameter;
    }

    Buffer &getBuffer()
    {
        return mBuffer;
    }

    /** Copy the firmware parameter part to the supplied vector */
    void getFirmwareParameterContent(std::vector<uint8_t> &content) const
    {
        content.resize(mBuffer.getSize() - parameterOffset);
        auto startIterator = mBuffer.getElements().begin();
        std::copy(startIterator + parameterOffset, startIterator + mBuffer.getSize(),
            content.begin());
    }

    /** Copy the firmware parameter part from the supplied vector */
    void setFirmwareParameterContent(const std::vector<uint8_t> &content)
    {
        assert(content.size() == mBuffer.getSize() - parameterOffset);
        std::copy(content.begin(), content.end(),
            mBuffer.getElements().begin() + parameterOffset);
    }

private:
    BigCmdModuleAccessIoctlOutput & operator=(const BigCmdModuleAccessIoctlOutput&) = delete;

    /**
    * In a FW response, the parameter is not at the beginning of the buffer, but after a header
    * including driver status followed by FW status.
    */
    static const size_t parameterOffset = INTC_APP_MIN_BODY_SIZE
        + sizeof(driver::IoctlFwModuleParam)
        - MEMBER_SIZE(driver::IoctlFwModuleParam, module_parameter_data);

    /* Initialize pmointers to sub-structures */
    void initPointers()
    {
        /* Initializing member that points to the IoctlFwModuleParam structure */
        mModuleParameterAccess =
            reinterpret_cast<driver::IoctlFwModuleParam*>(mBuffer->Data.Parameter);

        /* Initializing member that points to the firmware parameter */
        mFirmwareParameter =
            reinterpret_cast<FirmwareParameterType*>(mModuleParameterAccess->module_parameter_data);
    }

    /** Calculate the overall size of the structure *
    * @param[in] firmwareParameterSize the size of the firmware parameter
    */
    static std::size_t calculateSize(std::size_t firmwareParameterSize)
    {
        return parameterOffset + firmwareParameterSize;
    }

    TypedBuffer<driver::Intc_App_Cmd_Body> mBuffer;
    driver::IoctlFwModuleParam *mModuleParameterAccess;
    FirmwareParameterType *mFirmwareParameter;
};

/** This class helps to calculate the allocation size of a dsp_fw::ModuleInfos structure
 * that contains dsp_fw::MaxModuleCount module entries */
class ModulesInfoHelper final
{
public:
    static std::size_t getAllocationSize(uint32_t moduleCount)
    {
        assert(moduleCount > 0);

        /* "moduleCount - 1" because the ModulesInfo structure contains one module
         * entry */
        return sizeof(dsp_fw::ModulesInfo)
            + (moduleCount - 1) * sizeof(dsp_fw::ModuleEntry);
    }

private:
    ModulesInfoHelper();
};

/** This class helps to construct an IOCTL_CMD_APP_TO_AUDIODSP_TINY_(GET|SET) structure
  * in order to set/get log parameters.
  *
  * The underlying driver type is Intc_App_TinyCmd, here is its structure:
  *
  * Intc_App_TinyCmd has 2 sub-structures:
  * -> Intc_App_Cmd_Header
  * -> Intc_App_Cmd_Body, this structure contains the payload, which is of type:
  *    -> FwLogState
  *
  * Note: the Intc_App_TinyCmd type has to be used as both input and output buffer of the
  * ioctl TinyGet and TinySet.
  */
class TinyCmdLogParameterIoctl
{
public:
    TinyCmdLogParameterIoctl() :
        mBuffer(calculateSize()), mFwLogState(nullptr)
    {
        /* Filling the header */
        driver::Intc_App_Cmd_Header &header = mBuffer.getContent().Header;
        header.FeatureID = static_cast<ULONG>(driver::FEATURE_FW_LOGS);
        header.ParameterID = parameterId;

        /* By contract, the DataSize has to be set with body size, i.e. the whole size minus
         * the header size. We cannot use directly use the body size
         * (i.e. sizeof(Intc_App_Cmd_Body)) because it depends on the parameter type. */
        header.DataSize = static_cast<ULONG>(calculateSize()
            - sizeof(driver::Intc_App_Cmd_Header));

        initPointers();
    }

    /** Copy constructor */
    TinyCmdLogParameterIoctl(const TinyCmdLogParameterIoctl &other) :
        mBuffer(other.mBuffer)
    {
        initPointers();
    }

    driver::Intc_App_TinyCmd &getTinyCmd()
    {
        return mBuffer.getContent();
    }

    driver::IoctlFwLogsState &getFwLogsState()
    {
        return *mFwLogState;
    }

    Buffer &getBuffer()
    {
        return mBuffer;
    }

private:

    /** By convention the parameter id of the "FEATURE_LOG_PARAMETERS" feature is 0 */
    static const uint32_t parameterId = 0;

    TinyCmdLogParameterIoctl & operator=(const TinyCmdLogParameterIoctl&) = delete;

    /** Init pointers to the sub-structures */
    void initPointers()
    {
        mFwLogState =
            reinterpret_cast<driver::IoctlFwLogsState*>(mBuffer.getContent().Body.Data.Parameter);
    }

    /** Calculate the overall size of the structure */
    static std::size_t calculateSize()
    {
        return sizeof(driver::Intc_App_TinyCmd) /* size of the whole structure */
            - MEMBER_SIZE(driver::Intc_App_Cmd_Body, Data) /* minus the parameter that points
                                                            * to the payload */
            + sizeof(driver::IoctlFwLogsState);   /* plus the payload size */
    }

    TypedBuffer<driver::Intc_App_TinyCmd> mBuffer;
    driver::IoctlFwLogsState *mFwLogState;
};

}
}
}

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

/** This class helps to construct IOCTL_CMD_APP_TO_AUDIODSP_BIG_(GET|SET) output structure.
 * This structure has 3 imbrication levels:
 * Intc_Get_Parameter (driver structure) which contains
 *   -> ModuleParameterAccess (driver structrue) which contains
 *        -> FirmwareParameterType (firmware structure), for instance AdspProperties,
 *           ModulesInfo...
 *
 * See Intel SST Driver Private IOCTL Specification (FDK Support).docx
 */
template <typename FirmwareParameterType>
class BigCmdIoctlOutput
{
public:
    /**
     * @param[in] fwParam The firmware parameter type
     * @param[in] firmwareParameterSize the size of the firmware parameter.
     *                                   Must be >= sizeof(FirmwareParameterType)
     */
    BigCmdIoctlOutput(dsp_fw::BaseFwParams fwParam, std::size_t firmwareParameterSize) :
        mBuffer(calculateSize(firmwareParameterSize)),
        mModuleParameterAccess(nullptr), mFirmwareParameter(nullptr)
    {
        /* Initializing member that points to the ModuleParameterAccess structure */
        mModuleParameterAccess =
            reinterpret_cast<driver::ModuleParameterAccess*>(mBuffer->Data.Parameter);

        /* Setting ModuleParameterAccess input properties */
        mModuleParameterAccess->ModuleId = baseFirwareModuleId;
        mModuleParameterAccess->InstanceId = baseFirwareInstanceId;
        mModuleParameterAccess->ModuleParameterId = static_cast<uint32_t>(fwParam);
        mModuleParameterAccess->ModuleParameterSize = static_cast<uint32_t>(firmwareParameterSize);

        /* Initializing member that points to the firmware parameter */
        mFirmwareParameter =
            reinterpret_cast<FirmwareParameterType*>(mModuleParameterAccess->Parameter);
    }

    /** @return Intc_App_Cmd_Body reference */
    driver::Intc_App_Cmd_Body &getCmdBody()
    {
        return mBuffer.getContent();
    }

    /** @return ModuleParameterAccess reference */
    driver::ModuleParameterAccess &getModuleParameterAccess()
    {
        return *mModuleParameterAccess;
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

private:
    static const uint32_t baseFirwareModuleId = 0;
    static const uint32_t baseFirwareInstanceId = 0;

    /** Calculate the overall size of the structure *
    * @param[in] firmwareParameterSize the size of the firmware parameter
    */
    static std::size_t calculateSize(std::size_t firmwareParameterSize)
    {
        return INTC_APP_MIN_BODY_SIZE  /* sizeof Intc_App_Cmd_Body minus payload pointer */
            + sizeof(driver::ModuleParameterAccess)
            - MEMBER_SIZE(driver::ModuleParameterAccess,Parameter)
            + firmwareParameterSize;
    }

    TypedBuffer<driver::Intc_App_Cmd_Body> mBuffer;
    driver::ModuleParameterAccess *mModuleParameterAccess;
    FirmwareParameterType *mFirmwareParameter;
};

/** This class helps to calculate the allocation size of a dsp_fw::ModuleInfos structure
 * that contains dsp_fw::MaxModuleCount module entries */
class ModulesInfoHelper final
{
public:
    static std::size_t getAllocationSize()
    {
        static_assert(dsp_fw::MaxModuleCount > 0, "MaxModuleCount should be greater than 0");

        /* "dsp_fw::MaxModuleCount - 1" because the ModulesInfo structure contains one module
         * entry */
        return sizeof(dsp_fw::ModulesInfo)
            + (dsp_fw::MaxModuleCount - 1) * sizeof(dsp_fw::ModuleEntry);
    }

private:
    ModulesInfoHelper();
};

}
}
}

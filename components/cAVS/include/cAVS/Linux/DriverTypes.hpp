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

#include "cAVS/DspFw/Common.hpp"
#include "cAVS/ModuleHandler.hpp"

namespace debug_agent
{
namespace cavs
{
namespace linux
{
namespace driver
{

struct Exception : std::runtime_error
{
    using std::runtime_error::runtime_error;
};

static const std::string wakeUpCoreCmd{"wake"};
static const std::string sleepCoreCmd{"sleep"};
static const std::string setGetCtrl{"/sys/kernel/debug/snd_soc_skl/modules/set_get_ctrl"};
static const std::string corePowerCtrl{"/sys/kernel/debug/snd_soc_skl/core_power"};

static const uint32_t LOADABLE_MODULE_ID = 0x1000;
static const uint32_t VENDOR_CONFIG_PARAM = 0xFF;

class CorePowerCommand
{
public:
    CorePowerCommand(bool isAllowedToSleep, unsigned int coreId)
        : mCoreId(coreId), mAllowedToSleep(isAllowedToSleep)
    {
    }

    util::Buffer getBuffer() const
    {
        std::string command((mAllowedToSleep ? sleepCoreCmd : wakeUpCoreCmd) + " " +
                            std::to_string(mCoreId));
        return {command.begin(), command.end()};
    }

private:
    unsigned int mCoreId;
    bool mAllowedToSleep;
};

static bool requireTunneledAccess(uint32_t moduleId, uint32_t paramId)
{
    return paramId != dsp_fw::BaseModuleParams::MOD_INST_PROPS ||
           (moduleId >= LOADABLE_MODULE_ID && paramId != dsp_fw::BaseModuleParams::MOD_INST_PROPS);
}

struct TunneledHeader
{
    uint32_t mParamId;
    uint32_t mParamSize;

    void fromStream(util::ByteStreamReader &reader)
    {
        reader.read(mParamId);
        reader.read(mParamSize);
    }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.write(mParamId);
        writer.write(mParamSize);
    }
};

struct MessageHeader
{
    MessageHeader(uint32_t cmd, uint32_t moduleId, uint32_t instanceId, uint32_t parameterId,
                  size_t parameterSize)
    {
        primary.type = cmd;
        primary.instanceId = instanceId;
        primary.moduleId = moduleId;
        primary.reserved = 0;

        extended.largeParamId = parameterId;
        // The size of the parameter I am expecting to read / write
        extended.dataDize = std::min(parameterSize, maxParameterPayloadSize);
        extended.reserved = 0;
    }

    union
    {
        struct
        {
            uint32_t moduleId : 16;
            uint32_t instanceId : 8;
            uint32_t type : 5;
            uint32_t reserved : 3;
        } primary;
        uint32_t primaryWord;
    };
    union
    {
        struct
        {
            uint32_t dataDize : 20;
            uint32_t largeParamId : 8;
            int32_t reserved : 4;
        } extended;
        uint32_t extendedWord;
    };

    // It is a no-op, the driver will never send back the message header.
    virtual void fromStream(util::ByteStreamReader &) {}

    virtual void toStream(util::ByteStreamWriter &writer) const
    {
        writer.write(primaryWord);
        writer.write(extendedWord);
    }
    virtual size_t getReplyPayloadSize() const { return extended.dataDize; }
};

struct TunneledMessageHeader : public MessageHeader
{
    TunneledMessageHeader(uint32_t cmd, uint32_t moduleId, uint32_t instanceId,
                          uint32_t parameterId, size_t parameterSize)
        : MessageHeader(cmd, moduleId, instanceId, VENDOR_CONFIG_PARAM,
                        parameterSize + sizeof(mTunneledHeader))
    {
        mTunneledHeader.mParamId = parameterId;
        mTunneledHeader.mParamSize =
            std::min(parameterSize + sizeof(mTunneledHeader), maxParameterPayloadSize);
    }
    void fromStream(util::ByteStreamReader &reader) override
    {
        MessageHeader::fromStream(reader);
        mTunneledHeader.fromStream(reader);
    }

    void toStream(util::ByteStreamWriter &writer) const override
    {
        MessageHeader::toStream(writer);
        mTunneledHeader.toStream(writer);
    }

    size_t getReplyPayloadSize() const override { return mTunneledHeader.mParamSize; }

    TunneledHeader mTunneledHeader;
};

class LargeConfigAccess
{
public:
    enum class CmdType : uint32_t
    {
        Get = 3,
        Set = 4
    };

    LargeConfigAccess(CmdType cmd, uint32_t moduleId, uint32_t instanceId, uint32_t parameterId,
                      size_t parameterSize, const util::Buffer &parameterPayload = {})
        : mParameterPayload(parameterPayload)
    {
        if (requireTunneledAccess(moduleId, parameterId)) {
            mHeader = std::make_unique<TunneledMessageHeader>((uint32_t)cmd, moduleId, instanceId,
                                                              parameterId, parameterSize);
        } else {
            mHeader = std::make_unique<MessageHeader>((uint32_t)cmd, moduleId, instanceId,
                                                      parameterId, parameterSize);
        }
        mSize = std::min(sizeof(*mHeader) + mParameterPayload.size(), maxParameterPayloadSize);
    }

    void fromStream(util::ByteStreamReader &reader) { mHeader->fromStream(reader); }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.write(mSize);
        mHeader->toStream(writer);
        writer.writeRawBuffer(mParameterPayload);
    }

    size_t getReplyPayloadSize() const { return mHeader->getReplyPayloadSize(); }

private:
    uint32_t mSize;
    std::unique_ptr<MessageHeader> mHeader;
    const util::Buffer mParameterPayload;
};

class ModuleConfigAccess
{
public:
    enum class CmdType : uint32_t
    {
        Set = 1,
        Get = 2,
    };

    ModuleConfigAccess(CmdType cmd, uint32_t moduleId, uint8_t instanceId, uint32_t paramIdData)
    {
        mMsg.primary.type = static_cast<uint32_t>(cmd);
        mMsg.primary.instanceId = instanceId;
        mMsg.primary.moduleId = moduleId;
        mMsg.primary.reserved = 0;

        mMsg.extended.paramIdData = paramIdData;
        mMsg.extended.reserved = 0;
        mSize = sizeof(mMsg);
    }

    void fromStream(util::ByteStreamReader &reader)
    {
        reader.read(mSize);
        reader.read(mMsg);
    }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.write(mSize);
        writer.write(mMsg);
    }

private:
    struct message
    {
        union
        {
            struct
            {
                uint32_t moduleId : 16;
                uint32_t instanceId : 8;
                uint32_t type : 5;
                uint32_t reserved : 3;
            } primary;
            uint32_t primaryWord;
        };
        union
        {
            struct
            {
                uint32_t paramIdData : 30;
                uint32_t reserved : 2;
            } extended;
            uint32_t extendedWord;
        };

        void fromStream(util::ByteStreamReader &reader)
        {
            reader.read(primaryWord);
            reader.read(extendedWord);
        }

        void toStream(util::ByteStreamWriter &writer) const
        {
            writer.write(primaryWord);
            writer.write(extendedWord);
        }
    } mMsg;

    uint32_t mSize;
};
}
}
}
}

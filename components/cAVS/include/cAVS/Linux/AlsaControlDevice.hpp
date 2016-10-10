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

#include "cAVS/Linux/ControlDevice.hpp"
#include "Util/ByteStreamReader.hpp"
#include "Util/ByteStreamWriter.hpp"

namespace private_driver
{
#include <alsa/asoundlib.h>
}

namespace debug_agent
{
namespace cavs
{
namespace linux
{

/** This class abstracts a Linux ALSA Device for control operation (ctl read, ctl write)
 */
class AlsaControlDevice final : public ControlDevice
{
private:
    /**
     * ALSA tlv Header
     */
    struct TlvHeader
    {
        TlvHeader(unsigned int tag, unsigned int length) : mTag(tag), mLength(length) {}
        unsigned int mTag;    /* control element numeric identification */
        unsigned int mLength; /* in bytes aligned to 4 */

        void fromStream(util::ByteStreamReader &reader)
        {
            reader.read(mTag);
            reader.read(mLength);
        }

        void toStream(util::ByteStreamWriter &writer) const
        {
            writer.write(mTag);
            writer.write(mLength);
        }
    };

public:
    /** @throw Device::Exception if the device initialization has failed */
    AlsaControlDevice(const std::string &name);

    void ctlRead(const std::string &name, util::Buffer &bufferOutput) override;
    void ctlWrite(const std::string &name, const util::Buffer &bufferInput) override;

    size_t getControlCountByTag(const std::string &name) const override;

private:
    void getCtlHandle(const std::string &name, private_driver::snd_ctl_t *&handle,
                      private_driver::snd_ctl_elem_id_t &id,
                      private_driver::snd_ctl_elem_info_t &info,
                      private_driver::snd_ctl_elem_value_t &control) const;

    static std::string getCtlName(const std::string &name)
    {
        return std::string{"name='" + name + "'"};
    }
    std::string mControl;
};
}
}
}

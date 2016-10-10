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

#include <cstddef>
#include "cAVS/Linux/TinyalsaControlDevice.hpp"
#include "Util/ByteStreamWriter.hpp"
#include "Util/ByteStreamReader.hpp"
#include "Util/convert.hpp"
#include <iostream>
#include <asoundlib.h>

using namespace debug_agent::util;
using namespace std;

namespace debug_agent
{
namespace cavs
{
namespace linux
{

TinyalsaControlDevice::TinyalsaControlDevice(const std::string &name) : ControlDevice(name)
{
    if (!convertTo(name, mControlId)) {
        throw Exception("Invalid control card.");
    }
}

void TinyalsaControlDevice::ctlRead(const std::string &name, util::Buffer &bufferOutput)
{
    struct mixer *mixer;
    struct mixer_ctl *control;

    mixer = mixer_open(mControlId);
    if (!mixer) {
        throw Exception("Failed to open control " + getCardName());
    }

    control = mixer_get_ctl_by_name(mixer, name.c_str());
    if (!control) {
        mixer_close(mixer);
        throw Exception("Cannot find " + name + " + control mixer on " + getCardName());
    }

    mixer_ctl_type type = mixer_ctl_get_type(control);
    if (type == MIXER_CTL_TYPE_UNKNOWN) {
        mixer_close(mixer);
        throw Exception("Invalid type for " + name + " + control mixer on " + getCardName());
    }
    unsigned int count = mixer_ctl_get_num_values(control);

    switch (type) {
    case MIXER_CTL_TYPE_BOOL:
    case MIXER_CTL_TYPE_INT:
    case MIXER_CTL_TYPE_ENUM: {
        // No set array supported by tinyalsa for enumeration type.
        // All accessed as int in single get.
        int value;
        if ((value = mixer_ctl_get_value(control, 0)) < 0) {
            mixer_close(mixer);
            throw Exception("Cannot read the given element from control " + getCardName());
        }
        MemoryByteStreamWriter writer;
        writer.write(value);
        bufferOutput = writer.getBuffer();
    } break;
    case MIXER_CTL_TYPE_BYTE:
        bufferOutput.resize(count);
        if (mixer_ctl_get_array(control, bufferOutput.data(), count) < 0) {
            mixer_close(mixer);
            throw Exception("Cannot read the given element from control " + getCardName());
        }
        break;
    default:
        mixer_close(mixer);
        throw Exception("No support of get array for Control " + name);
    }
    mixer_close(mixer);
}

void TinyalsaControlDevice::ctlWrite(const std::string &name, const util::Buffer &bufferInput)
{
    struct mixer *mixer;
    struct mixer_ctl *control;

    mixer = mixer_open(mControlId);
    if (!mixer) {
        throw Exception("Failed to open control " + getCardName());
    }

    control = mixer_get_ctl_by_name(mixer, name.c_str());
    if (!control) {
        mixer_close(mixer);
        throw Exception("Cannot find " + name + " + control mixer on " + getCardName());
    }

    mixer_ctl_type type = mixer_ctl_get_type(control);
    if (type == MIXER_CTL_TYPE_UNKNOWN) {
        mixer_close(mixer);
        throw Exception("Invalid type for " + name + " + control mixer on " + getCardName());
    }
    unsigned int count = mixer_ctl_get_num_values(control);

    MemoryByteStreamReader reader(bufferInput);
    switch (type) {
    case MIXER_CTL_TYPE_ENUM:
    case MIXER_CTL_TYPE_INT:
    case MIXER_CTL_TYPE_BOOL:
        // No set array supported by tinyalsa for enumeration type.
        // All accessed as int in single set.
        int typedVal;
        reader.read(typedVal);
        for (unsigned int idx = 0; idx < count; idx++) {
            mixer_ctl_set_value(control, idx, typedVal);
        }
        break;
    case MIXER_CTL_TYPE_BYTE:
        mixer_ctl_set_array(control, bufferInput.data(), bufferInput.size());
    default:
        mixer_close(mixer);
        throw Exception("No support of set value for Control " + name);
    }
    mixer_close(mixer);
}

size_t TinyalsaControlDevice::getControlCountByTag(const std::string &tag) const
{
    size_t count = 0;
    struct mixer *mixer;
    mixer = mixer_open(mControlId);
    if (!mixer) {
        throw Exception("Failed to open control " + getCardName());
    }
    unsigned int nbControls = mixer_get_num_ctls(mixer);
    for (unsigned int i = 0; i < nbControls; i++) {
        struct mixer_ctl *control = mixer_get_ctl(mixer, i);
        if (!control) {
            mixer_close(mixer);
            throw Exception("Cannot find control#" + std::to_string(i) + " on " + getCardName());
        }
        std::string name(mixer_ctl_get_name(control));
        if (name.find(tag) != std::string::npos) {
            count += 1;
        }
    }
    mixer_close(mixer);
    return count;
}
}
}
}

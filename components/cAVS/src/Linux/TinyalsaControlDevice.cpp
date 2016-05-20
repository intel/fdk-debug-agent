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

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

#include "cAVS/Linux/AlsaControlDevice.hpp"
#include "Util/ByteStreamWriter.hpp"
#include "Util/ByteStreamReader.hpp"
#include <iostream>

using namespace debug_agent::util;
using namespace std;

using namespace private_driver;

namespace debug_agent
{
namespace cavs
{
namespace linux
{

AlsaControlDevice::AlsaControlDevice(const std::string &name) : ControlDevice(name)
{
    int cardIndex = snd_card_get_index(getCardName().c_str());
    if (cardIndex < 0) {
        throw Exception("Invalid card " + getCardName() + " : " + snd_strerror(cardIndex));
    }
    mControl = {"hw:" + std::to_string(cardIndex)};
}

void AlsaControlDevice::getCtlHandle(const std::string &name, snd_ctl_t *&handle,
                                     snd_ctl_elem_id_t &id, snd_ctl_elem_info_t &info,
                                     snd_ctl_elem_value_t &control) const
{
    int error = snd_ctl_ascii_elem_id_parse(&id, name.c_str());
    if (error < 0) {
        throw Exception("Failed to translate " + name + " as a control: " + snd_strerror(error));
    }

    error = snd_ctl_open(&handle, mControl.c_str(), 0);
    if (error < 0) {
        throw Exception("Failed to open control " + mControl + ": " + snd_strerror(error));
    }

    snd_ctl_elem_info_set_id(&info, &id);
    error = snd_ctl_elem_info(handle, &info);
    if (error < 0) {
        snd_ctl_close(handle);
        throw Exception("Cannot find " + name + " control mixer on " + mControl + ": " +
                        snd_strerror(error));
    }
    snd_ctl_elem_info_get_id(&info, &id);

    snd_ctl_elem_value_set_id(&control, &id);

    if (!snd_ctl_elem_info_is_readable(&info)) {
        snd_ctl_close(handle);
        throw Exception("Control " + getCardName() + " element is unreadable ");
    }
    error = snd_ctl_elem_read(handle, &control);
    if (error < 0) {
        snd_ctl_close(handle);
        throw Exception("Cannot read the given element from control " + mControl + ":" +
                        std::string{snd_strerror(error)});
    }
}

void AlsaControlDevice::ctlRead(const std::string &name, util::Buffer &bufferOutput)
{
    const std::string ctlName{getCtlName(name)};
    snd_ctl_elem_id_t *id;
    snd_ctl_elem_info_t *info;
    snd_ctl_elem_id_alloca(&id);
    snd_ctl_elem_info_alloca(&info);
    snd_ctl_t *handle;
    snd_ctl_elem_value_t *control;
    snd_ctl_elem_value_alloca(&control);

    getCtlHandle(ctlName, handle, *id, *info, *control);
    assert(handle != NULL && info != NULL && id != NULL && control != NULL);

    snd_ctl_close(handle);

    unsigned int count = snd_ctl_elem_info_get_count(info);
    snd_ctl_elem_type_t type = snd_ctl_elem_info_get_type(info);

    MemoryByteStreamWriter writer;
    for (unsigned int idx = 0; idx < count; idx++) {
        switch (type) {
        case SND_CTL_ELEM_TYPE_BOOLEAN:
            writer.write(snd_ctl_elem_value_get_boolean(control, idx));
            std::cout << "values=" << snd_ctl_elem_value_get_boolean(control, idx) << std::endl;
            break;
        case SND_CTL_ELEM_TYPE_INTEGER:
            std::cout << "values=" << snd_ctl_elem_value_get_integer(control, idx) << std::endl;
            writer.write(snd_ctl_elem_value_get_integer(control, idx));
            break;
        case SND_CTL_ELEM_TYPE_INTEGER64:
            std::cout << "values=" << snd_ctl_elem_value_get_integer64(control, idx) << std::endl;
            writer.write(snd_ctl_elem_value_get_integer64(control, idx));
            break;
        case SND_CTL_ELEM_TYPE_ENUMERATED:
            std::cout << "values=" << snd_ctl_elem_value_get_enumerated(control, idx) << std::endl;
            writer.write(snd_ctl_elem_value_get_enumerated(control, idx));
            break;
        case SND_CTL_ELEM_TYPE_BYTES:
            std::cout << "values=" << snd_ctl_elem_value_get_byte(control, idx) << std::endl;
            writer.write(snd_ctl_elem_value_get_byte(control, idx));
            break;
        case SND_CTL_ELEM_TYPE_IEC958:
            snd_aes_iec958_t iec958;
            snd_ctl_elem_value_get_iec958(control, &iec958);
            std::cout << std::hex << "[AES0=" << iec958.status[0] << " AES1=" << iec958.status[1]
                      << " AES2=" << iec958.status[2] << " AES3=" << iec958.status[3] << std::endl;
            writer.write(iec958.status);
            break;
        default:
            throw Exception("Control " + getCardName() + " element has unknown type");
        }
    }
    bufferOutput = writer.getBuffer();
}

void AlsaControlDevice::ctlWrite(const std::string &name, const util::Buffer &bufferInput)
{
    const std::string ctlName{getCtlName(name)};
    snd_ctl_elem_id_t *id;
    snd_ctl_elem_info_t *info;
    snd_ctl_elem_id_alloca(&id);
    snd_ctl_elem_info_alloca(&info);
    snd_ctl_t *handle;
    snd_ctl_elem_value_t *control;
    snd_ctl_elem_value_alloca(&control);

    getCtlHandle(ctlName, handle, *id, *info, *control);
    assert(handle != NULL && info != NULL && id != NULL && control != NULL);

    unsigned int count = snd_ctl_elem_info_get_count(info);
    snd_ctl_elem_type_t type = snd_ctl_elem_info_get_type(info);

    MemoryByteStreamReader reader(bufferInput);
    for (unsigned int idx = 0; idx < count; idx++) {
        switch (type) {
        case SND_CTL_ELEM_TYPE_BOOLEAN:
            long booleanTypeVal;
            reader.read(booleanTypeVal);
            snd_ctl_elem_value_set_boolean(control, idx, booleanTypeVal);
            break;
        case SND_CTL_ELEM_TYPE_INTEGER:
            long integerTypeVal;
            reader.read(integerTypeVal);
            snd_ctl_elem_value_set_integer(control, idx, integerTypeVal);
            break;
        case SND_CTL_ELEM_TYPE_INTEGER64:
            long long integer64TypeVal;
            reader.read(integer64TypeVal);
            snd_ctl_elem_value_set_integer64(control, idx, integer64TypeVal);
            break;
        case SND_CTL_ELEM_TYPE_ENUMERATED:
            unsigned int enumeratedTypeVal;
            reader.read(enumeratedTypeVal);
            snd_ctl_elem_value_set_enumerated(control, idx, enumeratedTypeVal);
            break;
        case SND_CTL_ELEM_TYPE_BYTES:
            unsigned char bytesTypeVal;
            reader.read(bytesTypeVal);
            snd_ctl_elem_value_set_byte(control, idx, bytesTypeVal);
            break;
        case SND_CTL_ELEM_TYPE_IEC958:
            snd_aes_iec958_t iec958;
            reader.read(iec958.status);
            snd_ctl_elem_value_set_iec958(control, &iec958);
            break;
        default:
            snd_ctl_close(handle);
            throw Exception("Control " + getCardName() + " element has unknown type");
        }
    }
    int error = snd_ctl_elem_write(handle, control);
    if (error < 0) {
        snd_ctl_close(handle);
        throw Exception("Control " + mControl + " element write error:" +
                        std::string{snd_strerror(error)});
    }
    snd_ctl_close(handle);
}
}
}
}

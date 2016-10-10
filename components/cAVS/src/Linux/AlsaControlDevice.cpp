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

size_t AlsaControlDevice::getControlCountByTag(const std::string &tag) const
{
    size_t count = 0;
    snd_hctl_t *handle;
    snd_ctl_elem_id_t *id;
    snd_ctl_elem_info_t *info;
    snd_ctl_elem_id_alloca(&id);
    snd_ctl_elem_info_alloca(&info);

    int err = snd_hctl_open(&handle, mControl.c_str(), 0);
    if (err < 0) {
        throw Exception("Failed to open control " + mControl + ": " + snd_strerror(err));
    }
    err = snd_hctl_load(handle);
    if (err < 0) {
        snd_hctl_close(handle);
        throw Exception("Failed to load control " + mControl + ": " + snd_strerror(err));
    }
    snd_hctl_elem_t *elem = snd_hctl_first_elem(handle);
    while (elem != nullptr) {
        err = snd_hctl_elem_info(elem, info);
        if (err < 0) {
            snd_hctl_close(handle);
            throw Exception("Failed to get ctl elem info: " + mControl + ": " + snd_strerror(err));
        }
        snd_hctl_elem_get_id(elem, id);
        std::string name(snd_ctl_ascii_elem_id_get(id));
        if (name.find(tag) != std::string::npos) {
            count += 1;
        }
        elem = snd_hctl_elem_next(elem);
    }
    snd_hctl_close(handle);
    return count;
}

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

    unsigned int count = snd_ctl_elem_info_get_count(info);
    snd_ctl_elem_type_t type = snd_ctl_elem_info_get_type(info);

    MemoryByteStreamWriter writer;

    // Special hook for TLV Bytes Control
    if ((type == SND_CTL_ELEM_TYPE_BYTES) && snd_ctl_elem_info_is_tlv_readable(info)) {

        util::Buffer rawTlv(sizeof(TlvHeader) + count);
        int ret = snd_ctl_elem_tlv_read(handle, id, reinterpret_cast<unsigned int *>(rawTlv.data()),
                                        rawTlv.size());
        snd_ctl_close(handle);
        if (ret < 0) {
            throw Exception("Control " + getCardName() + " Unable to read element: " +
                            snd_strerror(ret));
        }
        const auto &dataBegin = begin(rawTlv) + sizeof(TlvHeader);
        bufferOutput = {dataBegin, dataBegin + count};
        return;
    }

    if (!snd_ctl_elem_info_is_readable(info)) {
        snd_ctl_close(handle);
        throw Exception("Control " + getCardName() + " element is unreadable ");
    }
    int error = snd_ctl_elem_read(handle, control);
    if (error < 0) {
        snd_ctl_close(handle);
        throw Exception("Cannot read the given element from control " + mControl + ":" +
                        std::string{snd_strerror(error)});
    }
    snd_ctl_close(handle);

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
    int error;

    getCtlHandle(ctlName, handle, *id, *info, *control);
    assert(handle != NULL && info != NULL && id != NULL && control != NULL);

    unsigned int count = snd_ctl_elem_info_get_count(info);
    snd_ctl_elem_type_t type = snd_ctl_elem_info_get_type(info);

    // Special hook for TLV Bytes Control
    if ((type == SND_CTL_ELEM_TYPE_BYTES) && snd_ctl_elem_info_is_tlv_writable(info)) {
        TlvHeader tlv(0, count);
        util::MemoryByteStreamWriter messageWriter;
        messageWriter.write(tlv);
        messageWriter.writeRawBuffer(bufferInput);
        util::Buffer rawTlv = messageWriter.getBuffer();

        error = snd_ctl_elem_tlv_write(handle, id, reinterpret_cast<unsigned int *>(rawTlv.data()));
        snd_ctl_close(handle);
        if (error < 0) {
            throw Exception("Control " + mControl + " element write error:" +
                            std::string{snd_strerror(error)});
        }
        return;
    }

    if (!snd_ctl_elem_info_is_writable(info)) {
        snd_ctl_close(handle);
        throw Exception("Control " + getCardName() + " element is unwritable ");
    }

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
    error = snd_ctl_elem_write(handle, control);
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

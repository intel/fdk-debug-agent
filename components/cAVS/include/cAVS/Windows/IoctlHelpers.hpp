/*
 * Copyright (c) 2015-2016, Intel Corporation
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

#include "cAVS/Windows/DriverTypes.hpp"
#include "cAVS/Windows/Device.hpp"
#include <tuple>
#include <Util/Buffer.hpp>
#include <utility>
#include <stdexcept>

namespace debug_agent
{
namespace cavs
{
namespace windows
{
/** This namespace helps to manipulate buffers of TinySet/Get and BigSet/Get ioctls
 *
 * There are two kind of ioctls:
 * - BigGet/BigSet: allow to set/get driver parameters using shared memory across user space
 *   and kernel space. Suitable for large parameter data.
 * - TinyGet/TinySet: allow to set/get driver parameters using memory copy across user space
 *   and kernel space. Suitable for small parameter data.
 *
 * A same parameter can manipulated by either BigGet/BigSet or either TinyGet/TinySet.
 * BigGet/BigSet is more efficient for big parameter data, TinyGet/TinySet is better for
 * small parameter data.
 *
 * Windows Ioctl involve two buffers: one input buffer, one output buffer (which is also used
 *as input!)
 *
 * Buffer construction for BigXXX ioctls:
 * - input buffer <- the header (driver::Intc_App_Cmd_Header structure)
 * - outputbuffer <- the body ( driver::Intc_App_Cmd_Body structure)
 *
 * Buffer construction for TinyXXX ioctls:
 * - input buffer <- a buffer that contains the header followed by the body
 * - output buffer <- the same buffer (header + body).
 */
namespace ioctl_helpers
{
namespace details
{
/** Serialize a header */
static util::Buffer toHeaderBuffer(ULONG featureID, ULONG parameterID, ULONG bodySize)
{
    driver::Intc_App_Cmd_Header header(featureID, parameterID, bodySize);

    util::MemoryByteStreamWriter writer;
    writer.write(header);
    return writer.getBuffer();
}

/** Deserialize a header */
static void fromHeaderBuffer(util::ByteStreamReader &reader)
{
    driver::Intc_App_Cmd_Header header(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
    reader.read(header);
}

/** Serialize a body */
static util::Buffer toBodyBuffer(const util::Buffer &bodyPayload)
{
    driver::Intc_App_Cmd_Body body;

    util::MemoryByteStreamWriter writer;
    writer.write(body);
    writer.writeRawBuffer(bodyPayload);

    return writer.getBuffer();
}

/** Deserialize a body */
static std::pair<NTSTATUS, util::Buffer> fromBodyBuffer(util::MemoryByteStreamReader &reader)
{
    driver::Intc_App_Cmd_Body body;
    reader.read(body);

    return {body.Status,
            {reader.getBuffer().begin() + reader.getPointerOffset(), reader.getBuffer().end()}};
}
} // namespace details

/** Generate header and body buffers to perform a BigSet/Get ioctl
 *
 * The BigSet/Get ioctl requires two distinct buffers, a header buffer and a body buffer.
 *
 * @param[in] featureID the driver feature id
 * @param[in] parameterID the driver parameter id
 * @param[in] bodyPayload a buffer which is the body payload. Its content format depends
 *                        of the couple (featureID, parameterID)
 *
 * @returns a pair of buffers to be used with the ioctl call: 1) the header and 2) the body
 */
static std::pair<util::Buffer, util::Buffer> toBigCmdBuffers(ULONG featureID, ULONG parameterID,
                                                             const util::Buffer &bodyPayload)
{
    auto bodyBuffer = details::toBodyBuffer(bodyPayload);
    auto headerBuffer =
        details::toHeaderBuffer(featureID, parameterID, static_cast<ULONG>(bodyBuffer.size()));

    return {headerBuffer, bodyBuffer};
}

/** Parse a body buffer returned by the BigSet/Get ioctl
 *
 * @param[in] bodyBuffer the body buffer returned by the ioctl
 *
 * @returns a pair of: 1) the driver status and 2) a buffer with the body payload
 */
static std::pair<NTSTATUS, util::Buffer> fromBigCmdBodyBuffer(const util::Buffer &bodyBuffer)
{
    util::MemoryByteStreamReader reader(bodyBuffer);
    return details::fromBodyBuffer(reader);
}

/** Generate one buffers to perform a TinySet/Get ioctl
 *
 * The TinySet/Get ioctl requires one buffer that contains both header and body.
 *
 * @param[in] featureID the driver feature id
 * @param[in] parameterID the driver parameter id
 * @param[in] bodyPayload a buffer which is the body payload. Its content format depends
 *                        of the couple (featureID, parameterID)
 *
 * @returns the created buffer that should be used with ioctl call
 */
static util::Buffer toTinyCmdBuffer(ULONG featureID, ULONG parameterID,
                                    const util::Buffer &bodyPayload)
{
    util::Buffer headerBuffer;
    util::Buffer bodyBuffer;
    std::tie(headerBuffer, bodyBuffer) = toBigCmdBuffers(featureID, parameterID, bodyPayload);

    util::MemoryByteStreamWriter writer;
    writer.writeRawBuffer(headerBuffer);
    writer.writeRawBuffer(bodyBuffer);
    return writer.getBuffer();
}

/** Parse a body buffer returned by the TinySet/Get ioctl
 *
 * @param[in] buffer the buffer returned by the ioctl
 *
 * @returns a pair of: 1) the driver status and 2) a buffer with the body payload
 */
static std::pair<NTSTATUS, util::Buffer> fromTinyCmdBuffer(const util::Buffer &buffer)
{
    util::MemoryByteStreamReader reader(buffer);
    details::fromHeaderBuffer(reader);
    return details::fromBodyBuffer(reader);
}

/** Send an ioctl to the driver.
 *
 * @see the other ioctl() function.
 */
template <class T, class Exception>
void ioctl(Device &device, driver::IoCtlType ioctlType, ULONG feature, ULONG parameterId, T &inout)
{
    using driver::to_string;
    using std::to_string;

    /* Creating the body payload using the IoctlFwLogsState type */
    util::MemoryByteStreamWriter bodyPayloadWriter;
    bodyPayloadWriter.write(inout);

    /* Creating the TinySet/Get ioctl buffer */
    util::Buffer buffer =
        ioctl_helpers::toTinyCmdBuffer(feature, parameterId, bodyPayloadWriter.getBuffer());

    try {
        device.ioControl(ioctlType, &buffer, &buffer);
    } catch (Device::Exception &e) {
        throw Exception(to_string(ioctlType) + " error: " + e.what());
    }

    NTSTATUS driverStatus;
    util::Buffer bodyPayloadBuffer;
    /* Parsing returned buffer */
    std::tie(driverStatus, bodyPayloadBuffer) = ioctl_helpers::fromTinyCmdBuffer(buffer);

    if (!NT_SUCCESS(driverStatus)) {
        throw Exception("Driver returns invalid status: " +
                        to_string(static_cast<uint32_t>(driverStatus)));
    }

    try {
        /* Reading structure from body payload */
        util::MemoryByteStreamReader reader(bodyPayloadBuffer);
        reader.read(inout);

        if (!reader.isEOS()) {
            throw Exception("Ioctl buffer has not been fully consumed, type=" +
                            to_string(ioctlType) + ", pointer=" +
                            to_string(reader.getPointerOffset()) + ", size=" +
                            to_string(reader.getBuffer().size()) + ", remaining=" +
                            to_string(reader.getBuffer().size() - reader.getBuffer().size()));
        }
    } catch (util::ByteStreamReader::Exception &e) {
        throw Exception("Cannot decode ioctl buffer: " + std::string(e.what()));
    }
}

/** Send an ioctl to the driver
 *
 * @tparam T A type describing the ioctl (id, direction, type of the data
 *           to be sent - described by a Data member).
 * @tparam Exception The type of exception to be thrown.
 *
 * @param[in] device The device on which to call the ioctl.
 * @param[in,out] inout Reference to the data to be sent/received.
 */
template <class T, class Exception>
void ioctl(Device &device, typename T::Data &inout)
{
    static_assert(T::type == driver::IoCtlType::TinyGet || T::type == driver::IoCtlType::TinySet,
                  "For now, windows::ioctl_helpers::ioctl only supports Tiny ioctls");
    ioctl<T::Data, Exception>(device, T::type, T::feature, T::id, inout);
}
} // namespace ioctl_helpers
}
}
}

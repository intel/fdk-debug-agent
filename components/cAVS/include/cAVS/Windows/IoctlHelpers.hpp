/*
 ********************************************************************************
 *                              INTEL CONFIDENTIAL
 *   Copyright(C) 2015-2016 Intel Corporation. All Rights Reserved.
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

#include "cAVS/Windows/DriverTypes.hpp"
#include <tuple>
#include <Util/Buffer.hpp>
#include <utility>

namespace debug_agent
{
namespace cavs
{
namespace windows
{

/** This class helps to manipulate buffers of TinySet/Get and BigSet/Get ioctls
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
class IoctlHelpers
{
public:
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
        auto bodyBuffer = toBodyBuffer(bodyPayload);
        auto headerBuffer =
            toHeaderBuffer(featureID, parameterID, static_cast<ULONG>(bodyBuffer.size()));

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
        return fromBodyBuffer(reader);
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
        fromHeaderBuffer(reader);
        return fromBodyBuffer(reader);
    }

private:
    IoctlHelpers();

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
};
}
}
}

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

#include "cAVS/Windows/DriverTypes.hpp"

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
     * @param[out] headerBuffer the created header buffer that should be used with ioctl call
     * @param[out] bodyBuffer the created body buffer that should be used with ioctl call
     */
    static void toBigCmdBuffers(ULONG featureID, ULONG parameterID, const util::Buffer &bodyPayload,
                                util::Buffer &headerBuffer, util::Buffer &bodyBuffer)
    {
        toBodyBuffer(bodyPayload, bodyBuffer);
        toHeaderBuffer(featureID, parameterID, static_cast<ULONG>(bodyBuffer.size()), headerBuffer);
    }

    /** Parse a body buffer returned by the BigSet/Get ioctl
     * @param[in] bodyBuffer the body buffer returned by the ioctl
     * @param[out] status the returned driver status
     * @param[out] bodyPayload a buffer that contains the body payload
     */
    static void fromBigCmdBodyBuffer(const util::Buffer &bodyBuffer, NTSTATUS &status,
                                     util::Buffer &bodyPayload)
    {
        util::ByteStreamReader reader(bodyBuffer);
        fromBodyBuffer(reader, status, bodyPayload);
    }

    /** Generate one buffers to perform a TinySet/Get ioctl
     *
     * The TinySet/Get ioctl requires one buffer that contains both header and body.
     *
     * @param[in] featureID the driver feature id
     * @param[in] parameterID the driver parameter id
     * @param[in] bodyPayload a buffer which is the body payload. Its content format depends
     *                        of the couple (featureID, parameterID)
     * @param[out] buffer the created buffer that should be used with ioctl call
     */
    static void toTinyCmdBuffer(ULONG featureID, ULONG parameterID, const util::Buffer &bodyPayload,
                                util::Buffer &buffer)
    {
        util::Buffer headerBuffer;
        util::Buffer BodyBuffer;
        toBigCmdBuffers(featureID, parameterID, bodyPayload, headerBuffer, BodyBuffer);

        util::ByteStreamWriter writer;
        writer.writeRawBuffer(headerBuffer);
        writer.writeRawBuffer(BodyBuffer);
        buffer = writer.getBuffer();
    }

    /** Parse a body buffer returned by the TinySet/Get ioctl
     * @param[in] buffer the buffer returned by the ioctl
     * @param[out] status the returned driver status
     * @param[out] bodyPayload a buffer that contains the body payload
     */
    static void fromTinyCmdBuffer(const util::Buffer &buffer, NTSTATUS &status,
                                  util::Buffer &bodyPayload)
    {
        util::ByteStreamReader reader(buffer);
        fromHeaderBuffer(reader);
        fromBodyBuffer(reader, status, bodyPayload);
    }

private:
    IoctlHelpers();

    /** Serialize a header */
    static void toHeaderBuffer(ULONG featureID, ULONG parameterID, ULONG bodySize,
                               util::Buffer &headerBuffer)
    {
        driver::Intc_App_Cmd_Header header(featureID, parameterID, bodySize);

        util::ByteStreamWriter writer;
        writer.write(header);
        headerBuffer = writer.getBuffer();
    }

    /** Deserialize a header */
    static void fromHeaderBuffer(util::ByteStreamReader &reader)
    {
        driver::Intc_App_Cmd_Header header(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
        reader.read(header);
    }

    /** Serialize a body */
    static void toBodyBuffer(const util::Buffer &bodyPayload, util::Buffer &bodyBuffer)
    {
        driver::Intc_App_Cmd_Body body;

        util::ByteStreamWriter writer;
        writer.write(body);
        writer.writeRawBuffer(bodyPayload);

        bodyBuffer = writer.getBuffer();
    }

    /** Deserialize a body */
    static void fromBodyBuffer(util::ByteStreamReader &reader, NTSTATUS &status,
                               util::Buffer &bodyPayload)
    {
        driver::Intc_App_Cmd_Body body;
        reader.read(body);
        status = body.Status;

        bodyPayload.clear();
        bodyPayload.insert(bodyPayload.begin(),
                           reader.getBuffer().begin() + reader.getPointerOffset(),
                           reader.getBuffer().end());
    }
};
}
}
}

/*
 * Copyright (c) 2015, Intel Corporation
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

#include <System/Streamer.hpp>
#include <System/IfdkStreamHeader.hpp>
#include <ostream>
#include <string>
#include <stdexcept>

namespace debug_agent
{
namespace system
{
/**
 * A IfdkStreamer streams a IFDK stream to an ostream in real time.
 */
class IfdkStreamer : public Streamer
{

protected:
    /**
     * @param[in] systemType the name of the system type that provides this stream
     * @param[in] formatType the name of the format type of this stream
     * @param[in] majorVersion the major version of the file type stream format
     * @param[in] minorVersion the minor version of the file type stream format
     * @remarks not public since has to be constructed only from subclasses.
     * @throw IfdkStreamHeader::Exception
     */
    IfdkStreamer(const std::string &systemType, const std::string &formatType,
                 unsigned int majorVersion, unsigned int minorVersion);

    /**
     * Add a property to the stream.
     * Allow subclasses to reach the generic header to add custom properties.
     * @see IfdkStreamHeader::addProperty
     * @throw IfdkStreamHeader::Exception
     */
    void addProperty(const std::string &key, const std::string &value);

    /**
     * Stream the Format specific header
     * @param[in] os the std::ostream on which the stream will be written to
     */
    virtual void streamFormatHeader(std::ostream &os) = 0;

    /**
     * Stream the next format data chunk
     * @param[in] os the std::ostream on which the stream will be written to
     * @return true if data are still available, false otherwise (end of stream)
     */
    virtual bool streamNextFormatData(std::ostream &os) = 0;

private:
    virtual void streamFirst(std::ostream &os) override final;

    virtual bool streamNext(std::ostream &os) override final;

    /**
     * Stream the IFDK generic header
     * @param[in] os the std::ostream on which the stream will be written to
     */
    void streamGenericHeader(std::ostream &os);

    /**
     * The stream generic header
     */
    IfdkStreamHeader mIfdkHeader;

    /* Make this class non copyable */
    IfdkStreamer(const IfdkStreamer &) = delete;
    IfdkStreamer &operator=(const IfdkStreamer &) = delete;
};
}
}
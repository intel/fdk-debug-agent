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
#include <System/IfdkStreamer.hpp>

namespace debug_agent
{
namespace system
{

IfdkStreamer::IfdkStreamer(const std::string &systemType, const std::string &formatType,
                           unsigned int majorVersion, unsigned int minorVersion)
    : mIfdkHeader(systemType, formatType, majorVersion, minorVersion)
{
}

void IfdkStreamer::streamFirst(std::ostream &os)
{
    /* IFDK header has first stream data */
    streamGenericHeader(os);
    /* Call subclass format specific header which comes after IFDK header */
    streamFormatHeader(os);
}

bool IfdkStreamer::streamNext(std::ostream &os)
{
    return streamNextFormatData(os);
}

void IfdkStreamer::streamGenericHeader(std::ostream &os)
{
    os << mIfdkHeader;
}

void IfdkStreamer::addProperty(const std::string &key, const std::string &value)
{
    mIfdkHeader.addProperty(key, value);
}
}
}

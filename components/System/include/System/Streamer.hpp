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

#include <ostream>
#include <string>
#include <stdexcept>

namespace debug_agent
{
namespace system
{
/**
 * A Streamer streams a raw stream to an ostream in real time.
 */
class Streamer
{
public:
    struct Exception final : std::logic_error
    {
        using std::logic_error::logic_error;
    };

    virtual ~Streamer(){};

    /**
     * This operator allows to streams out the Streamer's stream to the ostream.
     * @param[in] os the std::ostream on which the stream will be streamed to
     * @param[in] s the Streamer
     * @remarks Should be called from a dedicated thread since will block until stream termination
     * @throw Streamer::Exception
     */
    friend std::ostream &operator<<(std::ostream &os, Streamer &s);

protected:
    /**
     * @remarks not public since has to be constructed only from subclasses.
     */
    Streamer(){};

    /**
     * Request to stream out the first chunck of stream data.
     * This method is called once per stream, before streamNext().
     * Subclasses are free to override it in order to implement stream prologue such as header.
     * @param[in] os the ostream on which log stream has to be written to
     */
    virtual void streamFirst(std::ostream &os);

    /**
     * Request to stream out the next chunck of stream data.
     * This method is called in loop while it returns true.
     * @param[in] os the ostream on which log stream has to be written to
     * @return true if the method wants to be called again, false otherwise (end of stream)
     */
    virtual bool streamNext(std::ostream &os) = 0;

private:
    /**
     * This methods actually produces the real time stream which will be written to the
     * std::ostream.
     * The method will return once an error occurs on the stream write operation, or once an error
     * occurs to generate the stream.
     * @param[in] os the std::ostream on which the stream will be written to
     * @remarks Should be called from a dedicated thread
     * @throw streamer::Exception
     */
    void doStream(std::ostream &os);

    /* Make this class non copyable */
    Streamer(const Streamer &) = delete;
    Streamer &operator=(const Streamer &) = delete;
};
}
}

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
    class Exception final : public std::logic_error
    {
    public:
        explicit Exception(const std::string &what) : std::logic_error(what) {}
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
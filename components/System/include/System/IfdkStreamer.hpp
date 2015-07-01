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
    IfdkStreamer(const std::string &systemType,
                 const std::string &formatType,
                 unsigned int majorVersion,
                 unsigned int minorVersion);

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
    IfdkStreamer & operator=(const IfdkStreamer &) = delete;
};

}
}
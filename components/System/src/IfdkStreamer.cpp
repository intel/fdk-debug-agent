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
#include <System/IfdkStreamer.hpp>

namespace debug_agent
{
namespace system
{

IfdkStreamer::IfdkStreamer(const std::string &systemType,
                           const std::string &formatType,
                           unsigned int majorVersion,
                           unsigned int minorVersion) :
    mIfdkHeader(systemType, formatType, majorVersion, minorVersion)
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

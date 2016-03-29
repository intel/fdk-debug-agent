/*
 ********************************************************************************
 *                              INTEL CONFIDENTIAL
 *   Copyright(C) 2016 Intel Corporation. All Rights Reserved.
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

#include "cAVS/Linux/SystemDevice.hpp"
#include "cAVS/Linux/DriverTypes.hpp"

using namespace debug_agent::util;
using namespace std;

namespace debug_agent
{
namespace cavs
{
namespace linux
{

ssize_t SystemDevice::commandWrite(const std::string &name, const util::Buffer &bufferInput)
{
    std::lock_guard<std::mutex> locker(mClientMutex);
    ssize_t written;
    try {
        mFileHandler->open(name);
    } catch (const FileEntryHandler::Exception &e) {
        throw Exception("DebugFs handler returns an exception: " + std::string(e.what()));
    }
    try {
        written = mFileHandler->write(bufferInput);
    } catch (const FileEntryHandler::Exception &e) {
        mFileHandler->close();
        throw Exception("Failed to write command in file: " + name +
                        ", DebugFs handler returns an exception: " + std::string(e.what()));
    }
    mFileHandler->close();
    return written;
}

void SystemDevice::commandRead(const std::string &name, const util::Buffer &bufferInput,
                               util::Buffer &bufferOutput)
{
    std::lock_guard<std::mutex> locker(mClientMutex);
    try {
        mFileHandler->open(name);
    } catch (FileEntryHandler::Exception &e) {
        throw Exception("DebugFs handler returns an exception: " + std::string(e.what()));
    }

    /* Performing the debugfs write command, size ignored, as exception raised if partial write. */
    try {
        mFileHandler->write(bufferInput);
    } catch (const FileEntryHandler::Exception &e) {
        mFileHandler->close();
        throw Exception("Failed to write command in file: " + name +
                        ", DebugFs handler returns an exception: " + std::string(e.what()));
    }

    /* Reading the result of debugfs command read, size ignored as not meaningful info. */
    try {
        mFileHandler->read(bufferOutput, bufferOutput.size());
    } catch (const FileEntryHandler::Exception &e) {
        mFileHandler->close();
        throw Exception("Failed to read command answer from file: " + name +
                        ", DebugFs handler returns an exception: " + std::string(e.what()));
    }
    mFileHandler->close();
}
}
}
}

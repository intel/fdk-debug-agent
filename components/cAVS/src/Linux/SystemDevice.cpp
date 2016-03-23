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

void SystemDevice::setCorePowerState(unsigned int coreId, bool allowedToSleep)
{
    if (mDebugFsFile.is_open()) {
        throw Exception("Parallel operation not permitted.");
    }
    driver::CorePowerCommand corePowerCmd(allowedToSleep, coreId);
    try {
        debugfsOpen(driver::corePowerCtrl);
    } catch (const Device::Exception &e) {
        throw Exception("Device returns an exception: " + std::string(e.what()));
    }
    try {
        debugfsWrite(corePowerCmd.getBuffer());
    } catch (const Device::Exception &e) {
        debugfsClose();
        throw Exception("Get module parameter failed to write command IPC in file: " +
                        std::string(driver::corePowerCtrl) + ", Device returns an exception: " +
                        std::string(e.what()));
    }
    debugfsClose();
}

void SystemDevice::debugfsOpen(const std::string &name)
{
    if (mDebugFsFile.is_open()) {
        debugfsClose();
    }
    mDebugFsFile.exceptions(fstream::failbit | fstream::badbit | fstream::eofbit);
    try {
        mDebugFsFile.open(name, std::fstream::in | std::fstream::out);
    } catch (const std::exception &) {
        throw Exception("error while opening debugfs " + name + " file.");
    }
}

void SystemDevice::debugfsClose()
{
    try {
        mDebugFsFile.close();
    } catch (const std::exception &) {
        mDebugFsFile.clear();
    }
}

ssize_t SystemDevice::debugfsWrite(const Buffer &bufferInput)
{
    if (!mDebugFsFile.is_open()) {
        throw Exception("Illegal write operation on closed file");
    }
    try {
        mDebugFsFile.write(reinterpret_cast<const char *>(bufferInput.data()), bufferInput.size());
    } catch (const std::exception &) {
        throw Exception("error during write operation: " + mDebugFsFile.rdstate());
    }
    return bufferInput.size();
}

ssize_t SystemDevice::debugfsRead(Buffer &bufferOutput, const ssize_t nbBytes)
{
    if (!mDebugFsFile.is_open()) {
        throw Exception("Illegal read operation on closed file");
    }
    try {
        mDebugFsFile.seekg(0, ios_base::beg);
        mDebugFsFile.read(reinterpret_cast<char *>(bufferOutput.data()), nbBytes);
    } catch (const std::exception &) {
        if (mDebugFsFile.eof() && mDebugFsFile.gcount() > 0) {
            // Reading less than expected is not an error, as blind request with max size are ok.
            mDebugFsFile.clear();
        } else {
            throw Exception("error during read operation: " + mDebugFsFile.rdstate());
        }
    }
    return mDebugFsFile.gcount();
}
}
}
}

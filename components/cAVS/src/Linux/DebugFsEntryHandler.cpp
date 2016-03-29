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

#include "cAVS/Linux/DebugFsEntryHandler.hpp"

using namespace debug_agent::util;
using namespace std;

namespace debug_agent
{
namespace cavs
{
namespace linux
{

void DebugFsEntryHandler::open(const std::string &name)
{
    mFileEntry.exceptions(fstream::failbit | fstream::badbit | fstream::eofbit);
    if (mFileEntry.is_open()) {
        throw Exception("Parallel operation not permitted on same debugfs entry: " + name);
    }
    try {
        mFileEntry.open(name, std::fstream::in | std::fstream::out);
    } catch (const std::exception &) {
        throw Exception("error while opening debugfs " + name + " file.");
    }
}

void DebugFsEntryHandler::close() noexcept
{
    try {
        mFileEntry.close();
    } catch (const std::exception &) {
        mFileEntry.clear();
    }
}

ssize_t DebugFsEntryHandler::write(const Buffer &bufferInput)
{
    if (!mFileEntry.is_open()) {
        throw Exception("Illegal write operation on closed file");
    }
    try {
        mFileEntry.write(reinterpret_cast<const char *>(bufferInput.data()), bufferInput.size());
    } catch (const std::exception &) {
        throw Exception("error during write operation: " + mFileEntry.rdstate());
    }
    return bufferInput.size();
}

ssize_t DebugFsEntryHandler::read(Buffer &bufferOutput, const ssize_t nbBytes)
{
    if (!mFileEntry.is_open()) {
        throw Exception("Illegal read operation on closed file");
    }
    try {
        mFileEntry.seekg(0, ios_base::beg);
        mFileEntry.read(reinterpret_cast<char *>(bufferOutput.data()), nbBytes);
    } catch (const std::exception &) {
        if (mFileEntry.eof() && mFileEntry.gcount() > 0) {
            // Reading less than expected is not an error, as blind request with max size are ok.
            mFileEntry.clear();
        } else {
            throw Exception("error during read operation: " + mFileEntry.rdstate());
        }
    }
    return mFileEntry.gcount();
}
}
}
}

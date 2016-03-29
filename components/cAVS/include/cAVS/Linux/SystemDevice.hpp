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

#pragma once

#include "cAVS/Linux/Device.hpp"
#include "cAVS/Linux/FileEntryHandler.hpp"
#include "Util/AssertAlways.hpp"
#include <mutex>
#include <memory>

namespace debug_agent
{
namespace cavs
{
namespace linux
{

/** This device implementation uses the real open/read/write/close functions */
class SystemDevice final : public Device
{
public:
    SystemDevice(std::unique_ptr<FileEntryHandler> fileHandler)
        : mFileHandler(std::move(fileHandler))
    {
        ASSERT_ALWAYS(mFileHandler != nullptr);
    }

    ssize_t commandWrite(const std::string &name, const util::Buffer &bufferInput) override;
    void commandRead(const std::string &name, const util::Buffer &bufferInput,
                     util::Buffer &bufferOutput) override;

private:
    /* A device supports concurent file manipulation calls */
    std::mutex mClientMutex;

    std::unique_ptr<FileEntryHandler> mFileHandler;
};
}
}
}

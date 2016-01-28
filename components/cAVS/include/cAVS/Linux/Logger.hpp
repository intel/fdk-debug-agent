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

#include <cAVS/Logger.hpp>

namespace debug_agent
{
namespace cavs
{
namespace linux
{

/**
 * Implements the cavs::Logger interface for Linux cAVS driver API.
 */
class Logger final : public cavs::Logger
{
public:
    void setParameters(const Parameters &parameters) override;

    Parameters getParameters() override;

    std::unique_ptr<LogBlock> readLogBlock() override;
    void stop() noexcept override;

private:
    /**
     * While driver Debug FS interface is not available, parameters storage
     * is emulated using this internal member.
     * @todo remove it!
     */
    Parameters mDriverEmulationParameter;
};
}
}
}

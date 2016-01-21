/*
********************************************************************************
*                              INTEL CONFIDENTIAL
*   Copyright(C) 2015-2016 Intel Corporation. All Rights Reserved.
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

#include "Core/TypeModel.hpp"
#include "Core/InstanceModel.hpp"
#include "Core/ParameterDispatcher.hpp"
#include "cAVS/System.hpp"
#include "Rest/Server.hpp"
#include "Util/Locker.hpp"
#include "ParameterSerializer/ParameterSerializer.hpp"
#include <inttypes.h>
#include <memory>

namespace debug_agent
{
namespace core
{

/** The debug agent application class */
class DebugAgent final
{
public:
    /** @throw DebugAgent::Exception */
    DebugAgent(const cavs::DriverFactory &driverFactory, uint32_t port,
               const std::string &pfwConfig, bool serverIsVerbose = false,
               bool validationRequested = false);
    ~DebugAgent();

    struct Exception : std::logic_error
    {
        using std::logic_error::logic_error;
    };

private:
    DebugAgent(const DebugAgent &) = delete;
    DebugAgent &operator=(const DebugAgent &) = delete;

    std::shared_ptr<TypeModel> createTypeModel();
    static std::shared_ptr<ifdk_objects::instance::System> createSystemInstance();
    std::unique_ptr<rest::Dispatcher> createDispatcher();
    static std::vector<std::shared_ptr<ParameterApplier>> createParamAppliers(
        cavs::System &system,
        util::Locker<parameter_serializer::ParameterSerializer> &paramSerializer);

    cavs::System mSystem;
    std::shared_ptr<TypeModel> mTypeModel;
    std::shared_ptr<ifdk_objects::instance::System> mSystemInstance;
    util::Locker<std::shared_ptr<InstanceModel>> mInstanceModel;
    util::Locker<parameter_serializer::ParameterSerializer> mParameterSerializer;
    ParameterDispatcher mParamDispatcher;
    rest::Server mRestServer;
};
}
}

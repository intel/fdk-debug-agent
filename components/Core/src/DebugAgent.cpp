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

#include "Core/DebugAgent.hpp"
#include "Core/Resources.hpp"
#include "Core/DebugResources.hpp"
#include "Core/TypeModelConverter.hpp"
#include "Core/InstanceModelConverter.hpp"
#include "Core/ModuleResources.hpp"
#include "cAVS/System.hpp"
#include "Util/StringHelper.hpp"
#include <memory>
#include <exception>

using namespace debug_agent::rest;
using namespace debug_agent::util;

namespace debug_agent
{
namespace core
{

std::shared_ptr<TypeModel> DebugAgent::createTypeModel()
{
    try {
        TypeModelConverter converter(mSystem);
        return converter.createModel();
    } catch (BaseModelConverter::Exception &e) {
        throw Exception("Can not create type model: " + std::string(e.what()));
    }
}

std::shared_ptr<ifdk_objects::instance::System> DebugAgent::createSystemInstance()
{
    try {
        return InstanceModelConverter::createSystem();
    } catch (BaseModelConverter::Exception &e) {
        throw Exception("Can not create system instance: " + std::string(e.what()));
    }
}

std::unique_ptr<rest::Dispatcher> DebugAgent::createDispatcher()
{
    assert(mTypeModel != nullptr);
    assert(mSystemInstance != nullptr);

    std::unique_ptr<rest::Dispatcher> dispatcher = std::make_unique<rest::Dispatcher>();

    /* Log service (hardcoded urls)
     *
     * Note: putting these hardcoded urls before the real ones because otherwise real urls
     * will be invoked first, and will fail.
     */

    dispatcher->addResource("/type/cavs.fwlogs/control_parameters",
                            std::make_shared<LogServiceTypeControlParametersResource>());
    dispatcher->addResource("/instance/cavs.fwlogs/0/control_parameters",
                            std::make_shared<LogServiceInstanceControlParametersResource>(mSystem));
    dispatcher->addResource("/instance/cavs.fwlogs/0/streaming",
                            std::make_shared<LogServiceStreamResource>(mSystem));

    /* System */
    dispatcher->addResource("/type", std::make_shared<SystemTypeResource>(*mTypeModel));
    dispatcher->addResource("/instance",
                            std::make_shared<SystemInstanceResource>(*mSystemInstance));

    /* Other types*/
    dispatcher->addResource("/type/${type_name}", std::make_shared<TypeResource>(*mTypeModel));
    dispatcher->addResource("/instance/${type_name}",
                            std::make_shared<InstanceCollectionResource>(mInstanceModel));
    dispatcher->addResource("/instance/${type_name}/${instance_id}",
                            std::make_shared<InstanceResource>(mInstanceModel));

    /* Create one resource instance for each module type*/
    uint16_t moduleId = 0;
    for (const cavs::dsp_fw::ModuleEntry &entry : mSystem.getModuleEntries()) {
        std::string moduleName =
            StringHelper::getStringFromFixedSizeArray(entry.name, sizeof(entry.name));

        dispatcher->addResource(
            "/instance/cavs.module-" + moduleName + "/${instanceId}/control_parameters",
            std::make_shared<ControlParametersModuleInstanceResource>(mSystem, mParameterSerializer,
                                                                      moduleName, moduleId));

        dispatcher->addResource(
            "/type/cavs.module-" + moduleName + "/${instanceId}/control_parameters",
            std::make_shared<ControlParametersModuleTypeResource>(mSystem, mParameterSerializer,
                                                                  moduleName, moduleId));

        moduleId++;
    }

    /* Refresh special case*/
    dispatcher->addResource("/instance/cavs/0/refreshed",
                            std::make_shared<RefreshSubsystemResource>(mSystem, mInstanceModel));

    /* Debug resources */
    dispatcher->addResource("/internal/modules",
                            std::make_shared<ModuleListDebugResource>(mSystem));
    dispatcher->addResource("/internal/topology", std::make_shared<TopologyDebugResource>(mSystem));
    dispatcher->addResource("/internal/model", std::make_shared<ModelDumpDebugResource>(
                                                   *mTypeModel, *mSystemInstance, mInstanceModel));

    return dispatcher;
}

DebugAgent::DebugAgent(const cavs::DriverFactory &driverFactory, uint32_t port,
                       const std::string &pfwConfig, bool isVerbose) try :
    /* Order is important! */
    mSystem(driverFactory),
    mTypeModel(createTypeModel()),
    mSystemInstance(createSystemInstance()),
    mInstanceModel(nullptr),
    mParameterSerializer(pfwConfig),
    mRestServer(createDispatcher(), port, isVerbose) {
    assert(mTypeModel != nullptr);
    assert(mSystemInstance != nullptr);
} catch (rest::Dispatcher::InvalidUriException &e) {
    throw Exception("Invalid resource URI: " + std::string(e.what()));
} catch (rest::Server::Exception &e) {
    throw Exception("Rest server error : " + std::string(e.what()));
} catch (cavs::System::Exception &e) {
    throw Exception("System error: " + std::string(e.what()));
}

DebugAgent::~DebugAgent()
{
    /* This call will unblock all threads that consume system events (log...) */
    mSystem.stop();

    /* Then rest server destructor can terminate the http request threads gracefully */
}
}
}

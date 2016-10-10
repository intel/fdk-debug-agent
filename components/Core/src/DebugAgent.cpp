/*
 * Copyright (c) 2015-2016, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "Core/DebugAgent.hpp"
#include "Core/Resources.hpp"
#include "Core/DebugResources.hpp"
#include "Core/TypeModelConverter.hpp"
#include "Core/InstanceModelConverter.hpp"
#include "Core/ModuleParameterApplier.hpp"
#include "Core/SubsystemParameterApplier.hpp"
#include "Core/LogServiceParameterApplier.hpp"
#include "Core/PerfServiceParameterApplier.hpp"
#include "Core/ProbeServiceParameterApplier.hpp"
#include "Core/ProbeEndPointParameterApplier.hpp"
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

    /* Service-specific URLs
     */
    dispatcher->addResource("/instance/cavs.fwlogs/0/streaming",
                            std::make_shared<LogServiceStreamResource>(mSystem));

    dispatcher->addResource("/instance/cavs.probe.endpoint/${instance_id}/streaming",
                            std::make_shared<ProbeStreamResource>(mSystem));

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

    /* Parameters */
    dispatcher->addResource("/type/${type_name}/control_parameters",
                            std::make_shared<ParameterStructureResource>(mSystem, mParamDispatcher,
                                                                         ParameterKind::Control));
    dispatcher->addResource("/instance/${type_name}/${instance_id}/control_parameters",
                            std::make_shared<ParameterValueResource>(mSystem, mParamDispatcher,
                                                                     ParameterKind::Control));

    dispatcher->addResource("/type/${type_name}/info_parameters",
                            std::make_shared<ParameterStructureResource>(mSystem, mParamDispatcher,
                                                                         ParameterKind::Info));
    dispatcher->addResource(
        "/instance/${type_name}/${instance_id}/info_parameters",
        std::make_shared<ParameterValueResource>(mSystem, mParamDispatcher, ParameterKind::Info));

    /* Refresh special case*/
    dispatcher->addResource("/instance/cavs/0/refreshed",
                            std::make_shared<RefreshSubsystemResource>(mSystem, mInstanceModel));

    /* Debug resources */
    dispatcher->addResource("/internal/modules",
                            std::make_shared<ModuleListDebugResource>(mSystem));
    dispatcher->addResource("/internal/topology", std::make_shared<TopologyDebugResource>(mSystem));
    dispatcher->addResource("/internal/model", std::make_shared<ModelDumpDebugResource>(
                                                   *mTypeModel, *mSystemInstance, mInstanceModel));

    /* Version resource */
    dispatcher->addResource("/about", std::make_shared<AboutResource>());

    return dispatcher;
}

std::vector<std::shared_ptr<ParameterApplier>> DebugAgent::createParamAppliers(
    cavs::System &system, util::Locker<parameter_serializer::ParameterSerializer> &paramSerializer)
{
    return {
        std::make_shared<ModuleParameterApplier>(system, paramSerializer),
        std::make_shared<SubsystemParameterApplier>(system),
        std::make_shared<LogServiceParameterApplier>(system),
        std::make_shared<PerfServiceParameterApplier>(system),
        std::make_shared<ProbeServiceParameterApplier>(system),
        std::make_shared<ProbeEndPointParameterApplier>(system),
    };
}

DebugAgent::DebugAgent(const cavs::DriverFactory &driverFactory, uint32_t port,
                       const std::string &pfwConfig, bool isVerbose, bool validationRequested) try :
    /* Order is important! */
    mSystem(driverFactory),
    mTypeModel(createTypeModel()),
    mSystemInstance(createSystemInstance()),
    mInstanceModel(nullptr),
    mParameterSerializer(pfwConfig, validationRequested),
    mParamDispatcher(createParamAppliers(mSystem, mParameterSerializer)),
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

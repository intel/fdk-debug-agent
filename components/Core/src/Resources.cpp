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
#include "Core/Resources.hpp"
#include "Core/InstanceModelConverter.hpp"
#include "Rest/CustomResponse.hpp"
#include "Rest/StreamResponse.hpp"
#include "IfdkObjects/Xml/TypeDeserializer.hpp"
#include "IfdkObjects/Xml/TypeSerializer.hpp"
#include "IfdkObjects/Xml/InstanceDeserializer.hpp"
#include "IfdkObjects/Xml/InstanceSerializer.hpp"
#include "Util/convert.hpp"
#include <sstream>

using namespace debug_agent::rest;
using namespace debug_agent::cavs;
using namespace debug_agent::ifdk_objects;

namespace debug_agent
{
namespace core
{

static const std::string ContentTypeHtml("text/html");
static const std::string ContentTypeXml("text/xml");
/**
* @fixme use the content type specified by SwAS. Currently, the SwAS does not specify
* which content type shall be used. A request has been sent to get SwAS updated. Until that,
* the rational is:
*  - the content is application specific: usage of 'application/'
*  - the content type is vendor specific: usage of standard 'vnd.'
*  - knowing the resource is an IFDK file, the client can read the streamed IFDK header to
*    know which <subsystem>:<file format> the resource is.
* @remarks http://www.iana.org/assignments/media-types/media-types.xhtml
*/
static const std::string ContentTypeIfdkFile("application/vnd.ifdk-file");

/** This method returns the value of a node part of an XML document, based on an XPath expression
 *
 *  @param const Poco::XML::Document* the XML document to parse
 *  @param const std::string& a simplified XPath expression describing the location of the node
 *  in the XML tree
 *
 *  @returns a std::string corresponding to the value of an XML node
 */
inline static const std::string getNodeValueFromXPath(const Poco::XML::Document *document,
                                                      const std::string &url)
{
    Poco::XML::Node *node = document->getNodeByPath(url);
    if (node) {
        return Poco::XML::fromXMLString(node->innerText());
    } else {
        throw Response::HttpError(Response::ErrorStatus::BadRequest,
                                  "Invalid parameters format: node for path \"" + url +
                                      "\" not found");
    }
}

Resource::ResponsePtr SystemTypeResource::handleGet(const Request &)
{
    xml::TypeSerializer serializer;
    mTypeModel.getSystem()->accept(serializer);

    return std::make_unique<Response>(ContentTypeXml, serializer.getXml());
}

Resource::ResponsePtr SystemInstanceResource::handleGet(const Request &)
{
    xml::InstanceSerializer serializer;
    mSystemInstance.accept(serializer);

    return std::make_unique<Response>(ContentTypeXml, serializer.getXml());
}

Resource::ResponsePtr TypeResource::handleGet(const Request &request)
{
    std::string typeName = request.getIdentifierValue("type_name");
    std::shared_ptr<const type::Type> typePtr = mTypeModel.getType(typeName);

    if (typePtr == nullptr) {
        throw Response::HttpError(Response::ErrorStatus::BadRequest, "Unknown type: " + typeName);
    }

    xml::TypeSerializer serializer;
    typePtr->accept(serializer);

    return std::make_unique<Response>(ContentTypeXml, serializer.getXml());
}

Resource::ResponsePtr InstanceCollectionResource::handleGet(const Request &request)
{
    xml::InstanceSerializer serializer;
    std::string typeName = request.getIdentifierValue("type_name");

    {
        const auto guard = mInstanceModel.lock();
        auto handle = guard->get();
        if (handle == nullptr) {
            throw Response::HttpError(Response::ErrorStatus::InternalError,
                                      "Instance model is undefined.");
        }
        std::shared_ptr<const instance::BaseCollection> collection =
            handle->getCollection(typeName);

        /* check nullptr using get() to avoid any KW error */
        if (collection.get() == nullptr) {
            throw Response::HttpError(Response::ErrorStatus::BadRequest,
                                      "Unknown type: " + typeName);
        }

        collection->accept(serializer);
    }

    return std::make_unique<Response>(ContentTypeXml, serializer.getXml());
}

Resource::ResponsePtr InstanceResource::handleGet(const Request &request)
{
    xml::InstanceSerializer serializer;
    std::string typeName = request.getIdentifierValue("type_name");
    std::string instanceId = request.getIdentifierValue("instance_id");

    {
        auto guard = mInstanceModel.lock();
        auto handle = guard->get();
        if (handle == nullptr) {
            throw Response::HttpError(Response::ErrorStatus::InternalError,
                                      "Instance model is undefined.");
        }
        std::shared_ptr<const instance::Instance> instancePtr =
            handle->getInstance(typeName, instanceId);

        /* check nullptr using get() to avoid any KW error */
        if (instancePtr.get() == nullptr) {
            throw Response::HttpError(Response::ErrorStatus::BadRequest,
                                      "Unknown instance: type=" + typeName + " instance_id=" +
                                          instanceId);
        }

        instancePtr->accept(serializer);
    }

    return std::make_unique<Response>(ContentTypeXml, serializer.getXml());
}

Resource::ResponsePtr RefreshSubsystemResource::handlePost(const Request &)
{
    std::shared_ptr<InstanceModel> instanceModel;
    auto guard = mInstanceModel.lock();

    try {
        InstanceModelConverter converter(mSystem);
        instanceModel = converter.createModel();
    } catch (BaseModelConverter::Exception &e) {
        /* Topology retrieving has failed: invalidate the previous one */
        guard->reset();

        throw Response::HttpError(Response::ErrorStatus::InternalError,
                                  "Cannot refresh instance model: " + std::string(e.what()));
    }

    /* Apply new topology */
    *guard.get() = instanceModel;

    return std::make_unique<Response>();
}

Resource::ResponsePtr ParameterStructureResource::handleGet(const Request &request)
{
    std::string typeName = request.getIdentifierValue("type_name");
    std::string structure;
    try {
        structure = mParamDispatcher.getParameterStructure(typeName, mKind);
    } catch (ParameterDispatcher::UnsupportedException &e) {
        throw Response::HttpError(Response::ErrorStatus::NotFound, e.what());
    } catch (ParameterDispatcher::Exception &e) {
        throw Response::HttpError(Response::ErrorStatus::InternalError, e.what());
    }

    return std::make_unique<Response>(ContentTypeXml, structure);
}

Resource::ResponsePtr ParameterValueResource::handleGet(const Request &request)
{
    std::string typeName = request.getIdentifierValue("type_name");
    std::string instanceId = request.getIdentifierValue("instance_id");

    std::string value;
    try {
        value = mParamDispatcher.getParameterValue(typeName, mKind, instanceId);
    } catch (ParameterDispatcher::UnsupportedException &e) {
        throw Response::HttpError(Response::ErrorStatus::NotFound, e.what());
    } catch (ParameterDispatcher::Exception &e) {
        throw Response::HttpError(Response::ErrorStatus::InternalError, e.what());
    }

    return std::make_unique<Response>(ContentTypeXml, value);
}

Resource::ResponsePtr ParameterValueResource::handlePut(const Request &request)
{
    /* Can set only control parameters */
    if (mKind != ParameterKind::Control) {
        throw Response::HttpError(Response::ErrorStatus::NotFound,
                                  "Can set only control parameters");
    }

    std::string typeName = request.getIdentifierValue("type_name");
    std::string instanceId = request.getIdentifierValue("instance_id");

    try {
        mParamDispatcher.setParameterValue(typeName, ParameterKind::Control, instanceId,
                                           request.getRequestContentAsString());
    } catch (ParameterDispatcher::UnsupportedException &e) {
        throw Response::HttpError(Response::ErrorStatus::NotFound, e.what());
    } catch (ParameterDispatcher::Exception &e) {
        throw Response::HttpError(Response::ErrorStatus::InternalError, e.what());
    }

    return std::make_unique<Response>();
}

/** Http response that streams from a Prober::OutputStreamResource */
class StreamResponse : public CustomResponse
{
public:
    StreamResponse(const std::string &contentType,
                   std::unique_ptr<System::OutputStreamResource> streamResource)
        : CustomResponse(contentType), mStreamResource(std::move(streamResource))
    {
    }

    void doBodyResponse(std::ostream &out) override
    {
        try {
            mStreamResource->doWriting(out);
        } catch (System::Exception &e) {
            throw Response::HttpAbort(std::string("cAVS Log stream error: ") + e.what());
        }
    }

private:
    std::unique_ptr<System::OutputStreamResource> mStreamResource;
};

Resource::ResponsePtr LogServiceStreamResource::handleGet(const Request &)
{
    /** Acquiring the log stream resource */
    auto &&resource = mSystem.tryToAcquireLogStreamResource();
    if (resource == nullptr) {
        throw Response::HttpError(Response::ErrorStatus::Locked,
                                  "Logging stream resource is already used.");
    }

    return std::make_unique<StreamResponse>(ContentTypeIfdkFile, std::move(resource));
}

ProbeId ProbeStreamResource::getProbeId(const Request &request)
{
    std::string instanceId = request.getIdentifierValue("instance_id");
    ProbeId::RawType probeIndex;
    if (!convertTo(instanceId, probeIndex)) {
        throw Response::HttpError(Response::ErrorStatus::BadRequest,
                                  "The probe index '" + instanceId + "' is invalid");
    }
    return ProbeId(probeIndex);
}

Resource::ResponsePtr ProbeStreamResource::handleGet(const Request &request)
{
    ProbeId probeId = getProbeId(request);

    auto &&resource = mSystem.tryToAcquireProbeExtractionStreamResource(probeId);
    if (resource == nullptr) {
        throw Response::HttpError(Response::ErrorStatus::Locked,
                                  "Probe extraction resource #" +
                                      std::to_string(probeId.getValue()) + " is already used.");
    }

    return std::make_unique<StreamResponse>(ContentTypeIfdkFile, std::move(resource));
}

Resource::ResponsePtr ProbeStreamResource::handlePut(const Request &request)
{
    ProbeId probeId = getProbeId(request);

    auto &&resource = mSystem.tryToAcquireProbeInjectionStreamResource(probeId);
    if (resource == nullptr) {
        throw Response::HttpError(Response::ErrorStatus::Locked,
                                  "Probe injection resource #" +
                                      std::to_string(probeId.getValue()) + " is already used.");
    }

    try {
        resource->doReading(request.getRequestStream());
    } catch (System::Exception &e) {
        throw Response::HttpError(Response::ErrorStatus::InternalError,
                                  "Probe #" + std::to_string(probeId.getValue()) + " has failed: " +
                                      std::string(e.what()));
    }

    return std::make_unique<Response>();
}
}
}

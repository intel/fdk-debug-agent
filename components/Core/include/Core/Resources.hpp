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

/**
 * @fixme we should include IfdkObjects/type/Characteristics.hpp but because of a temporary issue
 *        we must include IfdkObjects/Xml/TypeTraits.hpp.
 */
#include "IfdkObjects/Xml/TypeTraits.hpp"
#include "Rest/Resource.hpp"
#include "cAVS/System.hpp"

namespace debug_agent
{
namespace core
{

class RootResource : public rest::Resource
{
public:
    explicit RootResource(cavs::System &system) : mSystem(system) {}
protected:
    cavs::System &mSystem;
};

/** This resource returns the uuid<->module_id mapping table */
class ModuleEntryResource : public RootResource
{
public:
    ModuleEntryResource(cavs::System &system) : RootResource(system) {}
protected:
    virtual void handleGet(const rest::Request &request, rest::Response &response) override;
};

/** @fixme remove this class when the switch to SystemTypeResource is effectively complete
*
*   This resource returns the Legacy System Type, containing Legacy Subsystems Types (XML).
*   This resource is temporary and only here because the current version of FDK Tool is
*   requesting it, instead of the SystemTypeResource below. */
class LegacySystemTypeResource : public RootResource
{
public:
    LegacySystemTypeResource(cavs::System &system) : RootResource(system) {}
protected:
    virtual void handleGet(const rest::Request &request, rest::Response &response) override;
};

/** This resource returns the System Type, containing Subsystems Types (XML) */
class SystemTypeResource : public RootResource
{
public:
    SystemTypeResource(cavs::System &system) : RootResource(system) {}
protected:
    virtual void handleGet(const rest::Request &request, rest::Response &response) override;
};

/** This resource returns the System instance, containing Subsystem instances (XML) */
class SystemInstanceResource : public RootResource
{
public:
    SystemInstanceResource(cavs::System &system) : RootResource(system) {}
protected:
    virtual void handleGet(const rest::Request &request, rest::Response &response) override;
};

/** This resource returns a subsystem type (XML) */
class SubsystemTypeResource : public RootResource
{
public:
    SubsystemTypeResource(cavs::System &system) : RootResource(system) {}
protected:
    virtual void handleGet(const rest::Request &request, rest::Response &response) override;
private:
    void addSystemCharacteristics(ifdk_objects::type::Characteristics &ch);
};

/** This resource returns a list of all the Instances of a Subsystem (XML) */
class SubsystemsInstancesListResource : public RootResource
{
public:
    SubsystemsInstancesListResource(cavs::System &system) : RootResource(system) {}
protected:
    virtual void handleGet(const rest::Request &request, rest::Response &response) override;
};

/** This resource returns one Instance of a Subsystem (XML) */
class SubsystemInstanceResource : public RootResource
{
public:
    SubsystemInstanceResource(cavs::System &system) : RootResource(system) {}
protected:
    virtual void handleGet(const rest::Request &request, rest::Response &response) override;
};

/** This resource returns the Log Parameters for an Instance of a Subsystem (XML) */
class SubsystemInstanceLogParametersResource : public RootResource
{
public:
    SubsystemInstanceLogParametersResource(cavs::System &system) : RootResource(system) {}
protected:
    virtual void handleGet(const rest::Request &request, rest::Response &response) override;
    virtual void handlePut(const rest::Request &request, rest::Response &response) override;
};

/** This resource returns the Log Parameters for a Subsystem Type (XML) */
class SubsystemTypeLogParametersResource : public RootResource
{
public:
    SubsystemTypeLogParametersResource(cavs::System &system) : RootResource(system) {}
protected:
    virtual void handleGet(const rest::Request &request, rest::Response &response) override;
};

/** This resource returns the Log Stream for a Subsystem Instance (XML) */
class SubsystemInstanceLogStreamResource : public RootResource
{
public:
    SubsystemInstanceLogStreamResource(cavs::System &system) : RootResource(system) {}
protected:
    virtual void handleGet(const rest::Request &request, rest::Response &response) override;
};

}
}



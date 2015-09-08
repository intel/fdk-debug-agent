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
#include "IfdkObjects/Xml/InstanceSerializer.hpp"
#include "IfdkObjects/Xml/InstanceDeserializer.hpp"
#include "catch.hpp"

using namespace debug_agent::ifdk_objects::instance;
using namespace debug_agent::ifdk_objects::xml;

void populateChildren(Children &children)
{
    InstanceRefCollection* instanceColl = new InstanceRefCollection("instances");
    instanceColl->add(InstanceRef("instance1","0"));

    ComponentRefCollection* compColl = new ComponentRefCollection("components");
    compColl->add(ComponentRef("comp1","1"));

    ServiceRefCollection* servColl = new ServiceRefCollection("services");
    servColl->add(ServiceRef("service1", "2"));

    children.add(instanceColl);
    children.add(compColl);
    children.add(servColl);
}

void populateParents(Parents &parents)
{
    parents.add(new InstanceRef("instance_type", "1"));
    parents.add(new ComponentRef("component_type", "2"));
    parents.add(new SubsystemRef("subsystem_type", "3"));
}

void populateInputs(Inputs &inputs)
{
    inputs.add(Input("id1", "format1"));
    inputs.add(Input("id2", "format2"));
}

void populateOutputs(Outputs &outputs)
{
    outputs.add(Output("id1", "format1"));
    outputs.add(Output("id2", "format2"));
    outputs.add(Output("id3", "format3"));
}

void populateLinks(Links &links)
{
    links.resize(2);

    Link &link1 = links.getElements()[0];
    link1.getFrom().setTypeName("type1");
    link1.getFrom().setInstanceId("2");
    link1.getFrom().setOutputId("3");
    link1.getTo().setTypeName("type2");
    link1.getTo().setInstanceId("4");
    link1.getTo().setInputId("5");

    Link &link2 = links.getElements()[1];
    link2.getFrom().setTypeName("type3");
    link2.getFrom().setInstanceId("5");
    link2.getFrom().setOutputId("1");
    link2.getTo().setTypeName("type4");
    link2.getTo().setInstanceId("6");
    link2.getTo().setInputId("2");
}

TEST_CASE("Instance serializer: InstanceRef")
{
    /* Serialize */
    InstanceRef instanceRef("type1","0");

    InstanceSerializer serializer;
    instanceRef.accept(serializer);

    std::string xml = serializer.getXml();
    CHECK(xml == "<instance Id=\"0\" Type=\"type1\"/>\n");

    /* Deserialize */
    InstanceDeserializer deserializer(xml);
    InstanceRef deserializedInstance;

    CHECK_NOTHROW(deserializedInstance.accept(deserializer));
    CHECK(deserializedInstance == instanceRef);
}

TEST_CASE("Instance serializer: ComponentRef")
{
    /* Serialize */
    ComponentRef compRef("comp_type","1");

    InstanceSerializer serializer;
    compRef.accept(serializer);

    std::string xml = serializer.getXml();
    CHECK(xml == "<component Id=\"1\" Type=\"comp_type\"/>\n");

    /* Deserialize */
    InstanceDeserializer deserializer(xml);
    ComponentRef deserializedInstance;

    CHECK_NOTHROW(deserializedInstance.accept(deserializer));
    CHECK(deserializedInstance == compRef);
}

TEST_CASE("Instance serializer: ServiceRef")
{
    /* Serialize */
    ServiceRef serviceRef("serv_type","3");

    InstanceSerializer serializer;
    serviceRef.accept(serializer);

    std::string xml = serializer.getXml();
    CHECK(xml == "<service Id=\"3\" Type=\"serv_type\"/>\n");

    /* Deserialize */
    InstanceDeserializer deserializer(xml);
    ServiceRef deserializedInstance;

    CHECK_NOTHROW(deserializedInstance.accept(deserializer));
    CHECK(deserializedInstance == serviceRef);
}

TEST_CASE("Instance serializer: SubsystemRef")
{
    /* Serialize */
    SubsystemRef subsystemRef("subsystem_type","5");

    InstanceSerializer serializer;
    subsystemRef.accept(serializer);

    std::string xml = serializer.getXml();
    CHECK(xml == "<subsystem Id=\"5\" Type=\"subsystem_type\"/>\n");

    /* Deserialize */
    InstanceDeserializer deserializer(xml);
    SubsystemRef deserializedInstance;

    CHECK_NOTHROW(deserializedInstance.accept(deserializer));
    CHECK(deserializedInstance == subsystemRef);
}

TEST_CASE("Instance serializer: instance ref collection")
{
    /* Serialize */
    InstanceRefCollection instanceRef("my_instance_ref_coll");
    instanceRef.add(InstanceRef("type1","0"));
    instanceRef.add(InstanceRef("type2","2"));

    InstanceSerializer serializer;
    instanceRef.accept(serializer);

    std::string xml = serializer.getXml();

    CHECK(xml ==
        "<collection Name=\"my_instance_ref_coll\">\n"
        "    <instance Id=\"0\" Type=\"type1\"/>\n"
        "    <instance Id=\"2\" Type=\"type2\"/>\n"
        "</collection>\n"
        );

    /* Deserialize */
    InstanceDeserializer deserializer(xml);
    InstanceRefCollection deserializedInstance;

    CHECK_NOTHROW(deserializedInstance.accept(deserializer));
    CHECK(deserializedInstance == instanceRef);
}

TEST_CASE("Instance serializer: compo ref collection")
{
    /* Serialize */
    ComponentRefCollection compoRef("my_comp_ref_coll");
    compoRef.add(ComponentRef("type1", "3"));
    compoRef.add(ComponentRef("type2", "5"));

    InstanceSerializer serializer;
    compoRef.accept(serializer);

    std::string xml = serializer.getXml();

    CHECK(xml ==
        "<component_collection Name=\"my_comp_ref_coll\">\n"
        "    <component Id=\"3\" Type=\"type1\"/>\n"
        "    <component Id=\"5\" Type=\"type2\"/>\n"
        "</component_collection>\n"
        );

    /* Deserialize */
    InstanceDeserializer deserializer(xml);
    ComponentRefCollection deserializedInstance;

    CHECK_NOTHROW(deserializedInstance.accept(deserializer));
    CHECK(deserializedInstance == compoRef);
}

TEST_CASE("Instance serializer: service ref collection")
{
    /* Serialize */
    ServiceRefCollection serviceRef("my_service_ref_coll");
    serviceRef.add(ServiceRef("type1", "9"));
    serviceRef.add(ServiceRef("type2", "2"));

    InstanceSerializer serializer;
    serviceRef.accept(serializer);

    std::string xml = serializer.getXml();

    CHECK(xml ==
        "<service_collection Name=\"my_service_ref_coll\">\n"
        "    <service Id=\"9\" Type=\"type1\"/>\n"
        "    <service Id=\"2\" Type=\"type2\"/>\n"
        "</service_collection>\n"
        );

    /* Deserialize */
    InstanceDeserializer deserializer(xml);
    ServiceRefCollection deserializedInstance;

    CHECK_NOTHROW(deserializedInstance.accept(deserializer));
    CHECK(deserializedInstance == serviceRef);
}

TEST_CASE("Instance serializer: subsystem ref collection")
{
    /* Serialize */
    SubsystemRefCollection subsystemRef("my_subsystem_ref_coll");
    subsystemRef.add(SubsystemRef("type1", "1"));
    subsystemRef.add(SubsystemRef("type2", "0"));

    InstanceSerializer serializer;
    subsystemRef.accept(serializer);

    std::string xml = serializer.getXml();

    CHECK(xml ==
        "<subsystem_collection Name=\"my_subsystem_ref_coll\">\n"
        "    <subsystem Id=\"1\" Type=\"type1\"/>\n"
        "    <subsystem Id=\"0\" Type=\"type2\"/>\n"
        "</subsystem_collection>\n"
        );

    /* Deserialize */
    InstanceDeserializer deserializer(xml);
    SubsystemRefCollection deserializedInstance;

    CHECK_NOTHROW(deserializedInstance.accept(deserializer));
    CHECK(deserializedInstance == subsystemRef);
}

TEST_CASE("Instance serializer: Info parameters")
{
    /* Serialize */
    InfoParameters params;

    InstanceSerializer serializer;
    params.accept(serializer);

    std::string xml = serializer.getXml();

    CHECK(xml == "<info_parameters/>\n");

    /* Deserialize */
    InstanceDeserializer deserializer(xml);
    InfoParameters deserializedInstance;

    CHECK_NOTHROW(deserializedInstance.accept(deserializer));
    CHECK(deserializedInstance == params);
}

TEST_CASE("Instance serializer: Control parameters")
{
    /* Serialize */
    ControlParameters params;

    InstanceSerializer serializer;
    params.accept(serializer);

    std::string xml = serializer.getXml();

    CHECK(xml == "<control_parameters/>\n");

    /* Deserialize */
    InstanceDeserializer deserializer(xml);
    ControlParameters deserializedInstance;

    CHECK_NOTHROW(deserializedInstance.accept(deserializer));
    CHECK(deserializedInstance == params);
}

TEST_CASE("Instance serializer: children")
{
    /* Serialize */
    Children children;
    populateChildren(children);

    InstanceSerializer serializer;
    children.accept(serializer);

    std::string xml = serializer.getXml();

    CHECK(xml ==
        "<children>\n"
        "    <collection Name=\"instances\">\n"
        "        <instance Id=\"0\" Type=\"instance1\"/>\n"
        "    </collection>\n"
        "    <component_collection Name=\"components\">\n"
        "        <component Id=\"1\" Type=\"comp1\"/>\n"
        "    </component_collection>\n"
        "    <service_collection Name=\"services\">\n"
        "        <service Id=\"2\" Type=\"service1\"/>\n"
        "    </service_collection>\n"
        "</children>\n"
        );

    /* Deserialize */
    InstanceDeserializer deserializer(xml);
    Children deserializedInstance;

    CHECK_NOTHROW(deserializedInstance.accept(deserializer));
    CHECK(deserializedInstance == children);
}

TEST_CASE("Instance serializer: parents")
{
    /* Serialize */
    Parents parents;
    populateParents(parents);

    InstanceSerializer serializer;
    parents.accept(serializer);

    std::string xml = serializer.getXml();

    CHECK(xml ==
        "<parents>\n"
        "    <instance Id=\"1\" Type=\"instance_type\"/>\n"
        "    <component Id=\"2\" Type=\"component_type\"/>\n"
        "    <subsystem Id=\"3\" Type=\"subsystem_type\"/>\n"
        "</parents>\n"
        );

    /* Deserialize */
    InstanceDeserializer deserializer(xml);
    Parents deserializedInstance;

    CHECK_NOTHROW(deserializedInstance.accept(deserializer));
    CHECK(deserializedInstance == parents);
}

TEST_CASE("Instance serializer: Instance")
{
    /* Serialize */
    Instance instance("my_instance_type","1");
    populateParents(instance.getParents());
    populateChildren(instance.getChildren());

    InstanceSerializer serializer;
    instance.accept(serializer);

    std::string xml = serializer.getXml();
    CHECK(xml ==
        "<instance Id=\"1\" Type=\"my_instance_type\">\n"
        "    <info_parameters/>\n"
        "    <control_parameters/>\n"
        "    <parents>\n"
        "        <instance Id=\"1\" Type=\"instance_type\"/>\n"
        "        <component Id=\"2\" Type=\"component_type\"/>\n"
        "        <subsystem Id=\"3\" Type=\"subsystem_type\"/>\n"
        "    </parents>\n"
        "    <children>\n"
        "        <collection Name=\"instances\">\n"
        "            <instance Id=\"0\" Type=\"instance1\"/>\n"
        "        </collection>\n"
        "        <component_collection Name=\"components\">\n"
        "            <component Id=\"1\" Type=\"comp1\"/>\n"
        "        </component_collection>\n"
        "        <service_collection Name=\"services\">\n"
        "            <service Id=\"2\" Type=\"service1\"/>\n"
        "        </service_collection>\n"
        "    </children>\n"
        "</instance>\n"
        );

    /* Deserialize */
    InstanceDeserializer deserializer(xml);
    Instance deserializedInstance;

    CHECK_NOTHROW(deserializedInstance.accept(deserializer));
    CHECK(deserializedInstance == instance);
}

TEST_CASE("Instance serializer: Instance collection")
{
    /* Serialize */
    InstanceCollection collection;
    collection.add(new Instance("my_instance_type1", "1"));
    collection.add(new Instance("my_instance_type2", "2"));

    InstanceSerializer serializer;
    collection.accept(serializer);

    std::string xml = serializer.getXml();
    CHECK(xml ==
        "<collection>\n"
        "    <instance Id=\"1\" Type=\"my_instance_type1\">\n"
        "        <info_parameters/>\n"
        "        <control_parameters/>\n"
        "        <parents/>\n"
        "        <children/>\n"
        "    </instance>\n"
        "    <instance Id=\"2\" Type=\"my_instance_type2\">\n"
        "        <info_parameters/>\n"
        "        <control_parameters/>\n"
        "        <parents/>\n"
        "        <children/>\n"
        "    </instance>\n"
        "</collection>\n"
        );

    /* Deserialize */
    InstanceDeserializer deserializer(xml);
    InstanceCollection deserializedInstance;

    CHECK_NOTHROW(deserializedInstance.accept(deserializer));
    CHECK(deserializedInstance == collection);
}

TEST_CASE("Instance serializer: Service")
{
    /* Serialize */
    Service service("my_service_type", "2", Service::Direction::Incoming);
    populateParents(service.getParents());
    populateChildren(service.getChildren());

    InstanceSerializer serializer;
    service.accept(serializer);

    std::string xml = serializer.getXml();
    CHECK(xml ==
        "<service Direction=\"Incoming\" Id=\"2\" Type=\"my_service_type\">\n"
        "    <info_parameters/>\n"
        "    <control_parameters/>\n"
        "    <parents>\n"
        "        <instance Id=\"1\" Type=\"instance_type\"/>\n"
        "        <component Id=\"2\" Type=\"component_type\"/>\n"
        "        <subsystem Id=\"3\" Type=\"subsystem_type\"/>\n"
        "    </parents>\n"
        "    <children>\n"
        "        <collection Name=\"instances\">\n"
        "            <instance Id=\"0\" Type=\"instance1\"/>\n"
        "        </collection>\n"
        "        <component_collection Name=\"components\">\n"
        "            <component Id=\"1\" Type=\"comp1\"/>\n"
        "        </component_collection>\n"
        "        <service_collection Name=\"services\">\n"
        "            <service Id=\"2\" Type=\"service1\"/>\n"
        "        </service_collection>\n"
        "    </children>\n"
        "</service>\n"
        );

    /* Deserialize */
    InstanceDeserializer deserializer(xml);
    Service deserializedInstance;

    CHECK_NOTHROW(deserializedInstance.accept(deserializer));
    CHECK(deserializedInstance == service);
}

TEST_CASE("Instance serializer: Service collection")
{
    /* Serialize */
    ServiceCollection collection;
    collection.add(new Service("my_service_type1", "1", Service::Direction::Incoming));
    collection.add(new Service("my_service_type2", "2", Service::Direction::Outgoing));

    InstanceSerializer serializer;
    collection.accept(serializer);

    std::string xml = serializer.getXml();
    CHECK(xml ==
        "<service_collection>\n"
        "    <service Direction=\"Incoming\" Id=\"1\" Type=\"my_service_type1\">\n"
        "        <info_parameters/>\n"
        "        <control_parameters/>\n"
        "        <parents/>\n"
        "        <children/>\n"
        "    </service>\n"
        "    <service Direction=\"Outgoing\" Id=\"2\" Type=\"my_service_type2\">\n"
        "        <info_parameters/>\n"
        "        <control_parameters/>\n"
        "        <parents/>\n"
        "        <children/>\n"
        "    </service>\n"
        "</service_collection>\n"
        );

    /* Deserialize */
    InstanceDeserializer deserializer(xml);
    ServiceCollection deserializedInstance;

    CHECK_NOTHROW(deserializedInstance.accept(deserializer));
    CHECK(deserializedInstance == collection);
}

TEST_CASE("Instance serializer: Input")
{
    /* Serialize */
    Input connector("my_id", "my_format");

    InstanceSerializer serializer;
    connector.accept(serializer);

    std::string xml = serializer.getXml();

    CHECK(xml == "<input Format=\"my_format\" Id=\"my_id\"/>\n");

    /* Deserialize */
    InstanceDeserializer deserializer(xml);
    Input deserializedInstance;

    CHECK_NOTHROW(deserializedInstance.accept(deserializer));
    CHECK(deserializedInstance == connector);
}

TEST_CASE("Instance serializer: Output")
{
    /* Serialize */
    Output connector("my_id", "my_format");

    InstanceSerializer serializer;
    connector.accept(serializer);

    std::string xml = serializer.getXml();
    CHECK(xml == "<output Format=\"my_format\" Id=\"my_id\"/>\n");

    /* Deserialize */
    InstanceDeserializer deserializer(xml);
    Output deserializedInstance;

    CHECK_NOTHROW(deserializedInstance.accept(deserializer));
    CHECK(deserializedInstance == connector);
}

TEST_CASE("Instance serializer: Inputs")
{
    /* Serialize */
    Inputs connectors;
    populateInputs(connectors);

    InstanceSerializer serializer;
    connectors.accept(serializer);

    std::string xml = serializer.getXml();
    CHECK(xml ==
        "<inputs>\n"
        "    <input Format=\"format1\" Id=\"id1\"/>\n"
        "    <input Format=\"format2\" Id=\"id2\"/>\n"
        "</inputs>\n"
        );

    /* Deserialize */
    InstanceDeserializer deserializer(xml);
    Inputs deserializedInstance;

    CHECK_NOTHROW(deserializedInstance.accept(deserializer));
    CHECK(deserializedInstance == connectors);
}

TEST_CASE("Instance serializer: Outputs")
{
    /* Serialize */
    Outputs connectors;
    populateOutputs(connectors);

    InstanceSerializer serializer;
    connectors.accept(serializer);

    std::string xml = serializer.getXml();
    CHECK(xml ==
        "<outputs>\n"
        "    <output Format=\"format1\" Id=\"id1\"/>\n"
        "    <output Format=\"format2\" Id=\"id2\"/>\n"
        "    <output Format=\"format3\" Id=\"id3\"/>\n"
        "</outputs>\n"
        );

    /* Deserialize */
    InstanceDeserializer deserializer(xml);
    Outputs deserializedInstance;

    CHECK_NOTHROW(deserializedInstance.accept(deserializer));
    CHECK(deserializedInstance == connectors);
}

TEST_CASE("Instance serializer: From")
{
    /* Serialize */
    From from("type", "1", "2");

    InstanceSerializer serializer;
    from.accept(serializer);

    std::string xml = serializer.getXml();

    CHECK(xml == "<from Id=\"1\" OutputId=\"2\" Type=\"type\"/>\n");

    /* Deserialize */
    InstanceDeserializer deserializer(xml);
    From deserializedInstance;

    CHECK_NOTHROW(deserializedInstance.accept(deserializer));
    CHECK(deserializedInstance == from);
}

TEST_CASE("Instance serializer: To")
{
    /* Serialize */
    To to("type", "2", "3");

    InstanceSerializer serializer;
    to.accept(serializer);

    std::string xml = serializer.getXml();

    CHECK(xml == "<to Id=\"2\" InputId=\"3\" Type=\"type\"/>\n");

    /* Deserialize */
    InstanceDeserializer deserializer(xml);
    To deserializedInstance;

    CHECK_NOTHROW(deserializedInstance.accept(deserializer));
    CHECK(deserializedInstance == to);
}

TEST_CASE("Instance serializer: Link")
{
    /* Serialize */
    Link link;
    link.getFrom().setTypeName("type1");
    link.getFrom().setInstanceId("2");
    link.getFrom().setOutputId("3");
    link.getTo().setTypeName("type2");
    link.getTo().setInstanceId("4");
    link.getTo().setInputId("5");

    InstanceSerializer serializer;
    link.accept(serializer);

    std::string xml = serializer.getXml();

    CHECK(xml ==
        "<link>\n"
        "    <from Id=\"2\" OutputId=\"3\" Type=\"type1\"/>\n"
        "    <to Id=\"4\" InputId=\"5\" Type=\"type2\"/>\n"
        "</link>\n");

    /* Deserialize */
    InstanceDeserializer deserializer(xml);
    Link deserializedInstance;

    CHECK_NOTHROW(deserializedInstance.accept(deserializer));
    CHECK(deserializedInstance == link);
}

TEST_CASE("Instance serializer: Links")
{
    /* Serialize */
    Links links;
    populateLinks(links);

    InstanceSerializer serializer;
    links.accept(serializer);

    std::string xml = serializer.getXml();

    CHECK(xml ==
        "<links>\n"
        "    <link>\n"
        "        <from Id=\"2\" OutputId=\"3\" Type=\"type1\"/>\n"
        "        <to Id=\"4\" InputId=\"5\" Type=\"type2\"/>\n"
        "    </link>\n"
        "    <link>\n"
        "        <from Id=\"5\" OutputId=\"1\" Type=\"type3\"/>\n"
        "        <to Id=\"6\" InputId=\"2\" Type=\"type4\"/>\n"
        "    </link>\n"
        "</links>\n");

    /* Deserialize */
    InstanceDeserializer deserializer(xml);
    Links deserializedInstance;

    CHECK_NOTHROW(deserializedInstance.accept(deserializer));
    CHECK(deserializedInstance == links);
}

TEST_CASE("Instance serializer: Component")
{
    /* Serialize */
    Component compo("my_comp_type","2");
    populateParents(compo.getParents());
    populateChildren(compo.getChildren());
    populateInputs(compo.getInputs());
    populateOutputs(compo.getOutputs());
    populateLinks(compo.getLinks());

    InstanceSerializer serializer;
    compo.accept(serializer);

    std::string xml = serializer.getXml();
    CHECK(xml ==
        "<component Id=\"2\" Type=\"my_comp_type\">\n"
        "    <info_parameters/>\n"
        "    <control_parameters/>\n"
        "    <parents>\n"
        "        <instance Id=\"1\" Type=\"instance_type\"/>\n"
        "        <component Id=\"2\" Type=\"component_type\"/>\n"
        "        <subsystem Id=\"3\" Type=\"subsystem_type\"/>\n"
        "    </parents>\n"
        "    <children>\n"
        "        <collection Name=\"instances\">\n"
        "            <instance Id=\"0\" Type=\"instance1\"/>\n"
        "        </collection>\n"
        "        <component_collection Name=\"components\">\n"
        "            <component Id=\"1\" Type=\"comp1\"/>\n"
        "        </component_collection>\n"
        "        <service_collection Name=\"services\">\n"
        "            <service Id=\"2\" Type=\"service1\"/>\n"
        "        </service_collection>\n"
        "    </children>\n"
        "    <inputs>\n"
        "        <input Format=\"format1\" Id=\"id1\"/>\n"
        "        <input Format=\"format2\" Id=\"id2\"/>\n"
        "    </inputs>\n"
        "    <outputs>\n"
        "        <output Format=\"format1\" Id=\"id1\"/>\n"
        "        <output Format=\"format2\" Id=\"id2\"/>\n"
        "        <output Format=\"format3\" Id=\"id3\"/>\n"
        "    </outputs>\n"
        "    <links>\n"
        "        <link>\n"
        "            <from Id=\"2\" OutputId=\"3\" Type=\"type1\"/>\n"
        "            <to Id=\"4\" InputId=\"5\" Type=\"type2\"/>\n"
        "        </link>\n"
        "        <link>\n"
        "            <from Id=\"5\" OutputId=\"1\" Type=\"type3\"/>\n"
        "            <to Id=\"6\" InputId=\"2\" Type=\"type4\"/>\n"
        "        </link>\n"
        "    </links>\n"
        "</component>\n"
        );

    /* Deserialize */
    InstanceDeserializer deserializer(xml);
    Component deserializedInstance;

    CHECK_NOTHROW(deserializedInstance.accept(deserializer));
    CHECK(deserializedInstance == compo);
}

TEST_CASE("Instance serializer: ComponentCollection")
{
    /* Serialize */
    ComponentCollection collection;
    collection.add(new Component("my_comp_type1", "1"));
    collection.add(new Component("my_comp_type2", "2"));

    InstanceSerializer serializer;
    collection.accept(serializer);

    std::string xml = serializer.getXml();
    CHECK(xml ==
        "<component_collection>\n"
        "    <component Id=\"1\" Type=\"my_comp_type1\">\n"
        "        <info_parameters/>\n"
        "        <control_parameters/>\n"
        "        <parents/>\n"
        "        <children/>\n"
        "        <inputs/>\n"
        "        <outputs/>\n"
        "        <links/>\n"
        "    </component>\n"
        "    <component Id=\"2\" Type=\"my_comp_type2\">\n"
        "        <info_parameters/>\n"
        "        <control_parameters/>\n"
        "        <parents/>\n"
        "        <children/>\n"
        "        <inputs/>\n"
        "        <outputs/>\n"
        "        <links/>\n"
        "    </component>\n"
        "</component_collection>\n"
        );

    /* Deserialize */
    InstanceDeserializer deserializer(xml);
    ComponentCollection deserializedInstance;

    CHECK_NOTHROW(deserializedInstance.accept(deserializer));
    CHECK(deserializedInstance == collection);
}

TEST_CASE("Instance serializer: Subsystem")
{
    /* Serialize */
    Subsystem subsystem("my_subsystem_type", "3");
    populateParents(subsystem.getParents());
    populateChildren(subsystem.getChildren());
    populateInputs(subsystem.getInputs());
    populateOutputs(subsystem.getOutputs());
    populateLinks(subsystem.getLinks());

    InstanceSerializer serializer;
    subsystem.accept(serializer);

    std::string xml = serializer.getXml();
    CHECK(xml ==
        "<subsystem Id=\"3\" Type=\"my_subsystem_type\">\n"
        "    <info_parameters/>\n"
        "    <control_parameters/>\n"
        "    <parents>\n"
        "        <instance Id=\"1\" Type=\"instance_type\"/>\n"
        "        <component Id=\"2\" Type=\"component_type\"/>\n"
        "        <subsystem Id=\"3\" Type=\"subsystem_type\"/>\n"
        "    </parents>\n"
        "    <children>\n"
        "        <collection Name=\"instances\">\n"
        "            <instance Id=\"0\" Type=\"instance1\"/>\n"
        "        </collection>\n"
        "        <component_collection Name=\"components\">\n"
        "            <component Id=\"1\" Type=\"comp1\"/>\n"
        "        </component_collection>\n"
        "        <service_collection Name=\"services\">\n"
        "            <service Id=\"2\" Type=\"service1\"/>\n"
        "        </service_collection>\n"
        "    </children>\n"
        "    <inputs>\n"
        "        <input Format=\"format1\" Id=\"id1\"/>\n"
        "        <input Format=\"format2\" Id=\"id2\"/>\n"
        "    </inputs>\n"
        "    <outputs>\n"
        "        <output Format=\"format1\" Id=\"id1\"/>\n"
        "        <output Format=\"format2\" Id=\"id2\"/>\n"
        "        <output Format=\"format3\" Id=\"id3\"/>\n"
        "    </outputs>\n"
        "    <links>\n"
        "        <link>\n"
        "            <from Id=\"2\" OutputId=\"3\" Type=\"type1\"/>\n"
        "            <to Id=\"4\" InputId=\"5\" Type=\"type2\"/>\n"
        "        </link>\n"
        "        <link>\n"
        "            <from Id=\"5\" OutputId=\"1\" Type=\"type3\"/>\n"
        "            <to Id=\"6\" InputId=\"2\" Type=\"type4\"/>\n"
        "        </link>\n"
        "    </links>\n"
        "</subsystem>\n"
        );

    /* Deserialize */
    InstanceDeserializer deserializer(xml);
    Subsystem deserializedInstance;

    CHECK_NOTHROW(deserializedInstance.accept(deserializer));
    CHECK(deserializedInstance == subsystem);
}

TEST_CASE("Instance serializer: SubsystemCollection")
{
    /* Serialize */
    SubsystemCollection collection;
    collection.add(new Subsystem("subsystem_type1", "1"));
    collection.add(new Subsystem("subsystem_type2", "2"));

    InstanceSerializer serializer;
    collection.accept(serializer);

    std::string xml = serializer.getXml();
    CHECK(xml ==
        "<subsystem_collection>\n"
        "    <subsystem Id=\"1\" Type=\"subsystem_type1\">\n"
        "        <info_parameters/>\n"
        "        <control_parameters/>\n"
        "        <parents/>\n"
        "        <children/>\n"
        "        <inputs/>\n"
        "        <outputs/>\n"
        "        <links/>\n"
        "    </subsystem>\n"
        "    <subsystem Id=\"2\" Type=\"subsystem_type2\">\n"
        "        <info_parameters/>\n"
        "        <control_parameters/>\n"
        "        <parents/>\n"
        "        <children/>\n"
        "        <inputs/>\n"
        "        <outputs/>\n"
        "        <links/>\n"
        "    </subsystem>\n"
        "</subsystem_collection>\n"
        );

    /* Deserialize */
    InstanceDeserializer deserializer(xml);
    SubsystemCollection deserializedInstance;

    CHECK_NOTHROW(deserializedInstance.accept(deserializer));
    CHECK(deserializedInstance == collection);
}

TEST_CASE("Instance serializer: System")
{
    /* Serialize */
    System system("my_system_type", "4");

    SubsystemRefCollection *coll = new SubsystemRefCollection("subsystems");
    coll->add(SubsystemRef("subsystem1","1"));
    coll->add(SubsystemRef("subsystem2","2"));
    system.getChildren().add(coll);

    InstanceSerializer serializer;
    system.accept(serializer);

    std::string xml = serializer.getXml();
    CHECK(xml ==
        "<system Id=\"4\" Type=\"my_system_type\">\n"
        "    <info_parameters/>\n"
        "    <control_parameters/>\n"
        "    <parents/>\n"
        "    <children>\n"
        "        <subsystem_collection Name=\"subsystems\">\n"
        "            <subsystem Id=\"1\" Type=\"subsystem1\"/>\n"
        "            <subsystem Id=\"2\" Type=\"subsystem2\"/>\n"
        "        </subsystem_collection>\n"
        "    </children>\n"
        "    <inputs/>\n"
        "    <outputs/>\n"
        "    <links/>\n"
        "</system>\n"
        );

    /* Deserialize */
    InstanceDeserializer deserializer(xml);
    System deserializedInstance;

    CHECK_NOTHROW(deserializedInstance.accept(deserializer));
    CHECK(deserializedInstance == system);
}




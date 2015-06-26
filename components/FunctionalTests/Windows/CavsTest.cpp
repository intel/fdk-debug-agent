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

#include "Windows/IoctlHelper.hpp"
#include "Core/DebugAgent.hpp"
#include "TestCommon/HttpClientSimulator.hpp"
#include "cAVS/Windows/DeviceInjectionDriverFactory.hpp"
#include "cAVS/Windows/MockedDevice.hpp"
#include "catch.hpp"

using namespace debug_agent::core;
using namespace debug_agent::cavs;
using namespace debug_agent::test_common;


TEST_CASE("DebugAgent: module entries")
{
    /* Creating the mocked device */
    std::unique_ptr<windows::MockedDevice> device(new windows::MockedDevice());

    /* Filling the test vector */
    IoctlHelper::addModuleEntryCommand(*device);

    /* Creating the factory that will inject the mocked device */
    windows::DeviceInjectionDriverFactory driverFactory(std::move(device));

    /* Creating and starting the debug agent */
    DebugAgent debugAgent(driverFactory, 9090);

    /* Creating the http client */
    HttpClientSimulator client("localhost", 9090);

    std::string expectedContent(
        "<p>Module type count: 2</p>"
        "<table border='1'><tr><td>name</td><td>uuid</td><td>module id</td></tr>"
        "<tr><td>module_0</td><td>00000001000000020000000300000004</td><td>0</td></tr>"
        "<tr><td>module_1</td><td>0000000b0000000c0000000d0000000e</td><td>1</td></tr>"
        "</table>");

    /* Doing the request */
    client.request(
        "/cAVS/module/entries",
        HttpClientSimulator::Verb::Get,
        "",
        HttpClientSimulator::Status::Ok,
        "text/html",
        expectedContent
        );
}
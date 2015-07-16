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

#include "cAVS/Windows/RealTimeWppClient.hpp"
#include "cAVS/Windows/FileWppClient.hpp"
#include <Poco/Util/ServerApplication.h>
#include <atomic>
#include <thread>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <memory>

/** This executable tests the wpp classes by retrieving event logs
 *
 * If no parameter is supplied, a realtime session is used
 * If one parameter is supplied, it is used as input log file.
 */

using namespace debug_agent::cavs::windows;

class Application : public Poco::Util::ServerApplication
{
protected:
    virtual int main(const std::vector<std::string> &params) override
    {
        /* Instantiating the right wpp client */
        std::unique_ptr<WppClient> wppClient;
        if (params.empty()) {
            wppClient = std::move(std::make_unique<RealTimeWppClient>());
        }
        else if (params.size() == 1) {
            wppClient = std::move(std::make_unique<FileWppClient>(params[0]));
        }
        else {
            std::cout << "Usage: Loggraber [log input file]" << std::endl;
            return 1;
        }

        /* Starting a thread that will listen Ctrl+c in order to terminate the application */
        std::thread userExitRequestThread(
            &Application::waitForUserInterruption, this, std::ref(*wppClient));

        /* Note: if the user does Ctrl+c when the instruction pointer is here, WppClient::Stop()
         * will be called before WppClient::collectEntries(). But by contract this case is properly
         * handled : the WppClient::collectEntries() method will return immediately.
         *
         * @see WppClient::Stop()
         */

        std::cout
            << "LogGrabber" << std::endl
            << "----------" << std::endl
            << "Press Ctrl+C to stop." << std::endl << std::endl;

        int result = 0;

        try
        {
            LogEntryListener listener;
            wppClient->collectLogEntries(listener);
        }
        catch (WppClient::Exception &e)
        {
            std::cout << "WppClient exception: " << e.what() << std::endl;
            result = 1;
        }

        /* Unblock the thread that is calling waitForTerminationRequest(); */
        terminate();

        /* Wait for thread end. */
        userExitRequestThread.join();

        return result;
    }

private:
    /** Only first bytes of buffers are printed */
    static const uint32_t maxPrintedBufferSize = 12;

    /** Listener that prints log entries */
    class LogEntryListener : public WppLogEntryListener
    {
    public:
        LogEntryListener()  {}

        virtual void onLogEntry(uint32_t coreId, uint8_t *buffer, uint32_t size) override
        {
            static int i = 0;
            std::cout << "Entry: coreId=" << coreId << " size=" << size
                << " buffer=" << toHex(buffer, size, maxPrintedBufferSize) << std::endl;
        }
    };

    /* This method is called in a dedicated thread */
    void waitForUserInterruption(WppClient &wppClient)
    {
        waitForTerminationRequest();
        wppClient.stop();
    }

    /** Print the first n bytes of a buffer into a string */
    static std::string toHex(uint8_t *buf, uint32_t size, uint32_t printedOnlySize)
    {
        std::stringstream ss;
        ss << std::hex << std::setfill('0');

        uint32_t count = std::min(printedOnlySize, size);
        for (int i = 0; i < count; ++i)
        {
            ss << std::setw(2) << static_cast<uint32_t>(buf[i]);
        }

        if (printedOnlySize < size) {
            ss << std::dec << " and " << (size - printedOnlySize) << " more...";
        }

        return ss.str();
    }
};

int main(int argc, char* argv[])
{
    Application application;
    return application.run(argc, argv);
}
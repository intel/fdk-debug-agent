/*
 * Copyright (c) 2015, Intel Corporation
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
        } else if (params.size() == 1) {
            wppClient = std::move(std::make_unique<FileWppClient>(params[0]));
        } else {
            std::cout << "Usage: Loggraber [log input file]" << std::endl;
            return 1;
        }

        /* Starting a thread that will listen Ctrl+c in order to terminate the application */
        std::thread userExitRequestThread(&Application::waitForUserInterruption, this,
                                          std::ref(*wppClient));

        /* Note: if the user does Ctrl+c when the instruction pointer is here, WppClient::Stop()
         * will be called before WppClient::collectEntries(). But by contract this case is properly
         * handled : the WppClient::collectEntries() method will return immediately.
         *
         * @see WppClient::Stop()
         */

        std::cout << "LogGrabber" << std::endl
                  << "----------" << std::endl
                  << "Press Ctrl+C to stop." << std::endl
                  << std::endl;

        int result = 0;

        try {
            LogEntryListener listener;
            wppClient->collectLogEntries(listener);
        } catch (WppClient::Exception &e) {
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
        LogEntryListener() {}

        virtual void onLogEntry(uint32_t coreId, uint8_t *buffer, uint32_t size) override
        {
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
        for (int i = 0; i < count; ++i) {
            ss << std::setw(2) << static_cast<uint32_t>(buf[i]);
        }

        if (printedOnlySize < size) {
            ss << std::dec << " and " << (size - printedOnlySize) << " more...";
        }

        return ss.str();
    }
};

int main(int argc, char *argv[])
{
    Application application;
    return application.run(argc, argv);
}
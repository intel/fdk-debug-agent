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

#include "cAVS/Windows/WppCommon.hpp"
#include "cAVS/Windows/WindowsTypes.hpp"
#include <evntrace.h>
#include <mutex>
#include <stdexcept>

namespace debug_agent
{
namespace cavs
{
namespace windows
{

/** This class pilots wpp real time log session.
 *
 * Produced events can be consumed using the WppConsumer class.
 *
 * Notes:
 * - when start() is called, if a previous session exists, it is stopped.
 * - this class is thread safe, start() and stop() can be called concurrently
 */
class WppController final
{
public:
    class Exception : public std::logic_error
    {
    public:
        Exception(const std::string &msg) : std::logic_error(msg.c_str()) {}
    };

    WppController();
    ~WppController();

    /* Start the session.
     *
     * if a previous session exists, it is stopped.
     * @throw WppController::Exception
     */
    void start();

    void stop() NOEXCEPT;

private:
    /* Values defined by
     * https://msdn.microsoft.com/en-us/library/windows/desktop/aa364160(v=vs.85).aspx
     *
     * (ClientContext member)
     */
    enum ClientContextValues
    {
        PerformanceCounter = 1,
        SystemTime = 2,
        CPUCycleCount = 3
    };

    /** Flag used to filter firmware logs
     *
     * Wpp use flags to specify the log source: (from SW\HDAudioOEDrv\HDAudioOEDrv\Log.h)
     *
     * #define WPP_CONTROL_GUIDS \
     *     WPP_DEFINE_CONTROL_GUID(\
     *     BusCtrlGuid, (b3a109ec, 1cb3, 4947, 95ed, 431033eeb1b4), \
     *     \
     *     WPP_DEFINE_BIT(TRACE_DRIVER)    \
     *     WPP_DEFINE_BIT(TRACE_QUEUE)             \
     *     WPP_DEFINE_BIT(OE_PIPENODE)             \
     *     WPP_DEFINE_BIT(OE_PIPELINE)             \
     *     WPP_DEFINE_BIT(OE_IFACE)                \
     *     WPP_DEFINE_BIT(OE_FW)                   \
     *     WPP_DEFINE_BIT(OE_HW)                   \
     *     ...
     *
     * Therefore the firmware flag is 6th bit of the mask: 1 << 5
     */
    static const ULONG fwLogFlag = 1 << 5;

    /** Wpp requires a structure that contains an EVENT_TRACE_PROPERTIES instance followed by
     * the session name, then the log file name.
     *
     * @see https://msdn.microsoft.com/en-us/library/windows/desktop/aa364160(v=vs.85).aspx
     * (description of member "BufferSize")
     */
    struct TraceProperties
    {
        EVENT_TRACE_PROPERTIES properties;

        /**
        * from https://msdn.microsoft.com/en-us/library/windows/desktop/aa364103(v=vs.85).aspx:
        *
        * "You can use the maximum session name (1024 characters) and maximum log file name
        * (1024 characters) lengths to calculate the buffer size and offsets if not known."
        */
        static const std::size_t sessionNameMaxLen = 1024;
        static const std::size_t logFileNameMaxLen = 1024;

        char sessionName[sessionNameMaxLen];
        char logFileName[logFileNameMaxLen];

        TraceProperties();

        /* Set initial structure content:
         * The whole content is firstly set to zero, then the properties.Wnode.BufferSize field
         * is initialized.
         */
        void init();
    };

    WppController(const WppController &) = delete;
    WppController &operator=(const WppController &) = delete;

    /* If a previous session exists, delete it. */
    void cleanupOldSession();

    /* Must be called in a locked context. */
    void stopLocked();

    TRACEHANDLE mHandle;
    std::mutex mHandleMutex;
    TraceProperties mTraceProperties;
};
}
}
}

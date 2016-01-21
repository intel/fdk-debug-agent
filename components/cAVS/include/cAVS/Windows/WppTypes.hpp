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

#include "cAVS/Windows/WindowsTypes.hpp"
#include <stdint.h>

namespace debug_agent
{
namespace cavs
{
namespace windows
{

/** This namespace contains types that are shared between both wpp consumer and controller */
namespace common_wpp_types
{

/** Name of a wpp session.
 * This name is used to know if a session is already running. */
static const char wppSessionName[] = "AudioDspFwLog";
}

/** This namespace contains types used by wpp consumer */
namespace consumer_wpp_types
{

/**
* Used to filter out all messages except the one carrying FW logs.
*
* Important note: This identifier is generated by the WPP preprocessor, and may change
* unexpectedly if the code of the driver is modified.
*
* Here is a comment extracted from this driver source:
* SW\HDAudioOEDrv\HDAudioOEDrv\OELogListener.cpp
*
* "WARNING!!! DO NOT MOVE THIS METHOD OR ADD ANY LOGGING CALLS BEFORE OR INSIDE THIS
* METHOD OR YOU WILL BREAK EXTERNAL TOOLS THAT EXPECT DoTraceMessage ID TO BE EQUAL TO 10.
*
* Tools capturing FW logs may (and do!) depend on WPP message id given to
* DoTraceMessage. This id is a value of a counter that is (seems to be) incremented
* with each new logging call encountered by WPP while processing a file top to bottom."
*
* Therefore as conclusion this identifier is not reliable, another solution should be
* investigated.
*/
static const USHORT fwLogEventDescriptorId = 10;

/**
* Header that mirrors the parameters supplied by wpp for each log entry.
*
* Here is the driver log entry call:
*
* VOID
* OELogListener::DumpFwLogBuffer(
* _In_ UINT32 _Id,
* _In_ UINT32 _Consumed,
* _In_ UINT32 _WritePosition,
* _In_ UINT8* _Start)
* {
*     DoTraceMessage(
*         OE_FW, "core id: %08x size: %d position: %d %!HEXDUMP! ",
*         _Id,
*         _Consumed,
*         _WritePosition,
*         LOG_LENSTR(_Consumed, (PCHAR)_Start));
* }
*
* As you can see the supplied parameters to the DoTraceMessage function are:
* - the core id (uint32)
* - consumed, which is the buffer size (uint32)
* - write position, which is also the buffer size (uint32)
* - a buffer ("HEXDUMP" token), which is serialized by wpp in this way:
*    - the buffer size (uint16)
*    - the buffer content
*/
#pragma pack(1)
struct FwLogEntry
{
    uint32_t coreId;        /* Supplied by the driver */
    uint32_t size;          /* Supplied by the driver */
    uint32_t position;      /* Supplied by the driver, currently is always equal to the member
                            * "size" */
    uint16_t wppBufferSize; /* Buffer size supplied by wpp */
    uint8_t buffer[1];      /* Buffer content */
};
#pragma pack()

/** GUID of the log provider */
static const GUID AudioDspProviderGuid = {
    0xDB264037, 0x6BA1, 0x4DC0, {0xAE, 0x16, 0x5C, 0x60, 0xAD, 0x47, 0x0E, 0xDD}};
}

/** This namespace contains types used by wpp controller */
namespace controller_wpp_types
{
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

/* GUID for audio dsp log control */
static const GUID AudioDspLogControlGuid = {
    0xB3A109EC, 0x1CB3, 0x4947, {0x95, 0xED, 0x43, 0x10, 0x33, 0xEE, 0xB1, 0xB4}};
}
}
}
}
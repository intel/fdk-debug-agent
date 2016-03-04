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
 * Include this header when a Windows type is needed, instead of Windows.h
 *
 * Indeed including both windows.h and Poco/Foundation.h leads to conflict, because
 * Poco/Foundation.h already includes windows.h in a specfic way.
 *
 * Therefore when windows types are needed (for instance when using a windows device),
 * it's better to include Poco/Foundation.h than windows.h. This header hides this detail.
 */
#include <Poco/Foundation.h>

/** Included for NTSTATUS type */
#include <bcrypt.h>

/** Included for ioctl type definition */
#include <winioctl.h>

/* Defining manually the "STATUS_SUCCESS" value of NTSTATUS type because this value is
 * defined in ntstatus.h, which is a driver space header. Therefore including both
 * windows.h (or Poco/Foundation.h) and ntstatus.h leads to conflict (macro redefinition).
 */
#define STATUS_SUCCESS static_cast<NTSTATUS>(0)

// For the same reasons, defining also manually the NT_SUCCESS macro that
/** @def NT_SUCCESS(Status) Checks if a NTSTATUS is successful.
 *
 * By convention successful values are >= 0
 */
#define NT_SUCCESS(Status) (static_cast<NTSTATUS>(Status) >= 0)

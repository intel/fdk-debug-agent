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

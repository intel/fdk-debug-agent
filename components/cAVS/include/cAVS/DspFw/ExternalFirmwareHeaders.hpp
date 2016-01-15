/*
********************************************************************************
*                              INTEL CONFIDENTIAL
*   Copyright(C) 2016 Intel Corporation. All Rights Reserved.
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

#include <stdint.h>

/* Importing fw headers into a "private" namespace to ensure that they
 * will not be used accidentally */
namespace private_fw
{

/* Fw headers
 * Note: don't change the order, or it may not compile
 */
#include <stream_type.h>
#include <ipc/module_initial_settings.h>
#include <audio_data_format.h>
#include <fw_manifest_common.h>
#include <ixc/debug_api_params.h>

/* This macro has to be defined to enable compilation, even on Linux! */
#define WINNT
#include <ixc/adsp_ixc_defs.h>
#undef WINNT

#include <ixc/basefw_config.h>
#include <ixc/module_config.h>
}
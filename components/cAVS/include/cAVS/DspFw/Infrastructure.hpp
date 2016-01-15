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

#include "cAVS/DspFw/Common.hpp"
#include "Util/WrappedRaw.hpp"
#include <inttypes.h>

namespace debug_agent
{
namespace cavs
{
namespace dsp_fw
{

static const uint32_t IXC_STATUS_BITS = 24;

enum class IxcStatus
{
    ADSP_IPC_SUCCESS = 0, ///< The operation was successful.

    ADSP_IPC_ERROR_INVALID_PARAM = 1,  ///< Invalid parameter was passed.
    ADSP_IPC_UNKNOWN_MESSAGE_TYPE = 2, ///< Uknown message type was received.

    ADSP_IPC_OUT_OF_MEMORY = 3, ///< No physical memory to satisfy the request.

    ADSP_IPC_BUSY = 4,            ///< The system or resource is busy.
    ADSP_IPC_PENDING = 5,         ///< The action was scheduled for processing.
    ADSP_IPC_FAILURE = 6,         ///< Critical error happened.
    ADSP_IPC_INVALID_REQUEST = 7, ///< Request can not be completed.

    ADSP_RSVD_8 = 8, ///< Formerly ADSP_STAGE_UNINITIALIZED.

    ADSP_IPC_INVALID_RESOURCE_ID = 9, ///< Required resource can not be found.

    ADSP_RSVD_10 = 10, ///< Formerly ADSP_SOURCE_NOT_STARTED.

    ASDP_IPC_OUT_OF_MIPS = 11, ///< No computation power to satisfy the request.

    ADSP_IPC_INVALID_RESOURCE_STATE = 12, ///< Required resource is in invalid state.

    ADSP_IPC_POWER_TRANSITION_FAILED = 13, ///< Request to change power state failed.

    ADSP_IPC_INVALID_MANIFEST = 14, ///< Manifest of loadable library seems to be
                                    ///< invalid

    /* Load/unload library specific codes */
    ADSP_IPC_IMR_TO_SMALL = 42, ///< IMR is to small to load library
    ADSP_RSVD_43 = 43,
    ADSP_IPC_CSE_VALIDATION_FAILED = 44, ///< CSE didn't verified library

    ADSP_IPC_MOD_MGMT_ERROR = 100,        ///< Errors related to module management.
    ADSP_IPC_MOD_LOAD_CL_FAILED = 101,    ///< Loading of a module segment failed.
    ADSP_IPC_MOD_LOAD_INVALID_HASH = 102, ///< Invalid content of module received, hash does
                                          ///< not match.

    ADSP_IPC_MOD_UNLOAD_INST_EXIST = 103, ///< Request to unload module when its instances
                                          ///< are still present.
    ADSP_IPC_MOD_NOT_INITIALIZED = 104,   ///< ModuleInstance was't initialized.
    ADSP_IPC_MOD_STATE_NOT_SET = 105,     ///< ModuleInstance state wasn't set correctly.

    ADSP_IPC_CONFIG_GET_ERROR = 106,       ///< ModuleInstance couldn't get config.
    ADSP_IPC_CONFIG_SET_ERROR = 107,       ///< ModuleInstance couldn't set config.
    ADSP_IPC_LARGE_CONFIG_GET_ERROR = 108, ///< Failed to retrieve parameter from a module.
    ADSP_IPC_LARGE_CONFIG_SET_ERROR = 109, ///< Failed to apply parameter to a module.

    ADSP_IPC_MOD_INVALID_ID = 110,       ///< Passed Module ID is invalid (out of range).
    ADSP_IPC_MOD_INST_INVALID_ID = 111,  ///< Passed Instance ID is invalid (out of range
                                         ///< or not exist).
    ADSP_IPC_QUEUE_INVALID_ID = 112,     ///< Passed src queue ID is invalid (out of range).
    ADSP_IPC_QUEUE_DST_INVALID_ID = 113, ///< Passed dst queue ID is invalid (out of range).
    ADSP_IPC_BIND_UNBIND_DST_SINK_UNSUPPORTED = 114, ///< Bind/Unbind operation is not supported.
    ADSP_IPC_MOD_UNLOAD_INST_EXISTS = 115,           ///< ModuleInstance is using resources that are
                                                     ///< requested to remove .
    ADSP_IPC_CORE_INVALID_ID = 116,                  ///< Passed code ID is invalid.

    ADSP_IPC_INVALID_CONFIG_PARAM_ID = 120,    ///< Invalid configuration parameter ID specified.
    ADSP_IPC_INVALID_CONFIG_DATA_LEN = 121,    ///< Invalid length of configuration parameter
                                               ///< specified.
    ADSP_IPC_INVALID_CONFIG_DATA_STRUCT = 122, ///< Invalid structure of configuration parameter
                                               ///< specified.

    ADSP_IPC_GATEWAY_NOT_INITIALIZED = 140, ///< Gateway initialization error.
    ADSP_IPC_GATEWAY_NOT_EXIST = 141,       ///< Invalid gateway connector ID specified.
    ADSP_IPC_GATEWAY_STATE_NOT_SET = 142,   ///< Failed to change state of the gateway.

    ADSP_IPC_SCLK_ALREADY_RUNNING = 150, ///< SCLK can be configured when no I2S instance
                                         ///< is running.
    ADSP_IPC_MCLK_ALREADY_RUNNING = 151, ///< MCLK can be configured when no I2S instance
                                         ///< is running.
    ADSP_IPC_NO_RUNNING_SCLK = 152,      ///< SCLK can be stopped when at least 1 I2S
                                         ///< instance is running.
    ADSP_IPC_NO_RUNNING_MCLK = 153,      ///< MCLK can be stopped when at least 1 I2S
                                         ///< instance is running.

    ADSP_IPC_PIPELINE_NOT_INITIALIZED = 160, ///< Pipeline initialization error.
    ADSP_IPC_PIPELINE_NOT_EXIST = 161,       ///< Invalid pipeline ID specified.
    ADSP_IPC_PIPELINE_SAVE_FAILED = 162,     ///< Pipeline save operation failed.
    ADSP_IPC_PIPELINE_RESTORE_FAILED = 163,  ///< Pipeline restore operation failed.
    ADSP_IPC_PIPELINE_STATE_NOT_SET = 164,   ///< Failed to change state of the pipeline.
    ADSP_IPC_PIPELINE_ALREADY_EXISTS = 165,  ///< Pipeline with passed ID already exists in the
                                             ///< system.

    ADSP_IPC_MAX_STATUS = ((1 << IXC_STATUS_BITS) - 1)
};

enum class BaseFwParams
{
    /**
     * Use LARGE_CONFIG_GET to retrieve Adsp properties as TLV structure with type
     * of AdspProperties.
     * */
    ADSP_PROPERTIES = 0,
    /**
     * Use LARGE_CONFIG_GET to retrieve resources states (@see PhysMemPages).
     * */
    ADSP_RESOURCE_STATE = 1,
    /**
     * Use LARGE_CONFIG_SET to prepare core(s) for new DX state. Ipc mailbox must
     * contain properly built DxStateInfo struct. Power flows are described in FW HLD.
     * */
    DX_STATE = 2,
    /**
     * Use LARGE_CONFIG_SET to mask/unmask notifications. Ipc mailbox must contain properly
     * built NotificationMaskInfo struct.
     * */
    NOTIFICATION_MASK = 3,
    /**
     * Use LARGE_CONFIG_SET to setup new astate table. Ipc mailbox must contain properly built
     * AstateTable struct.
     * */
    ASTATE_TABLE = 4,
    /**
     * Use LARGE_CONFIG_SET to initialize or modify DMA gateway configuration outside of a
     * stream lifetime.
     * */
    DMA_CONTROL = 5,
    /**
     * Use LARGE_CONFIG_SET to manage logs priorities. Ipc mailbox must contain properly built
     * LogStateInfo structure.
     * */
    ENABLE_LOGS = 6,
    /**
     * Use LARGE_CONFIG_SET/LARGE_CONFIG_GET to write/read FW configuration.
     */
    FW_CONFIG = 7,
    /**
     * Use LARGE_CONFIG_GET to read HW configuration.
     */
    HW_CONFIG_GET = 8,
    /**
     * Use LARGE_CONFIG_GET to read modules configuration.
     */
    MODULES_INFO_GET = 9,
    /**
     * Use LARGE_CONFIG_GET to read pipeline list.
     */
    PIPELINE_LIST_INFO_GET = 10,
    /**
     * Use LARGE_CONFIG_GET to read pipelines properties.
     */
    PIPELINE_PROPS_GET = 11,
    /**
     * Use SCHEDULERS_INFO_GET to read schedulers configuration.
     */
    SCHEDULERS_INFO_GET = 12,
    /**
     * Use LARGE_CONFIG_GET to read gateway configuration.
     */
    GATEWAYS_INFO_GET = 13,
    /**
     * Use LARGE_CONFIG_GET to get information on memory state.
     */
    MEMORY_STATE_INFO_GET = 14,
    /**
     * Use LARGE_CONFIG_GET to get information on power state.
     */
    POWER_STATE_INFO_GET = 15
};

static inline ParameterId toParameterId(BaseFwParams param)
{
    return ParameterId{static_cast<ParameterId::RawType>(param)};
}

namespace detail
{
struct ExtendedIdTrait
{
    using RawType = uint32_t;
};
} // namespace detail

/** Represent the id of a base fw sub component.
 * The base firmware has several module like components in it.
 * To address them, the ParameterId is splited in a type and an instance part.
 */
using BaseFwInstanceId = util::WrappedRaw<detail::ExtendedIdTrait>;

enum class BaseModuleParams
{
    /**
    * Handled inside LargeConfigGet of module instance
    */
    MOD_INST_PROPS = 0xFE,
    /**
    * Handled inside ConfigSet of module instance
    */
    MOD_INST_ENABLE = 0x3000
};

static inline ParameterId toParameterId(BaseModuleParams param)
{
    return ParameterId{static_cast<ParameterId::RawType>(param)};
}
}
}
}

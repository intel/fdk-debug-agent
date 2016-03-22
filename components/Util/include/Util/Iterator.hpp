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

#include <iterator>

#ifdef _MSC_VER
#include <iterator>
/** Visual studio raises a warning if the check iterator feature is activated
 * but a raw pointer is used as iterator (as it can not check it's bounds).
 * As it is a safety feature, do not silent the warning, but use the
 * microsoft specific `make_check_array_iterator` that take a pointer
 * and the size of the underline buffer.
 * For other compiler, use the raw pointer.
 */
#define MAKE_ARRAY_ITERATOR(begin, size) stdext::make_checked_array_iterator(begin, size)
#else
/** By default an array iterator is a pointer to the first element. */
#define MAKE_ARRAY_ITERATOR(begin, size) begin
#endif

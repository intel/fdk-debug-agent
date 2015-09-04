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

#include <mutex>
#include <memory>

namespace debug_agent
{
namespace util
{

/* This class handles exclusive resources.
 *
 * When a client code needs the resource, it has to retrieve a handle. During
 * this handle life time, exclusive access is guaranteed.
 *
 * Usage example:
 *
 * struct MyResource {
 *     int resourceValue;
 * }
 *
 * // Creating the exclusive resource
 * ExclusiveResource<MyResource> resource;
 *
 * // beginning a block
 * {
 *     //getting the handle : resource is locked
 *     ExclusiveResource<MyResource>::HandlePtr resourceHandle = resource.acquireResource();
 *
 *     //using the resource, exclusive access is guaranteed
 *     resourceHandle->getResource().resourceValue = 2;
 *
 * // ending the block: the handle is destroyed and the resource is unlocked.
 * }
 */
template <typename T>
class ExclusiveResource final
{
public:
    ExclusiveResource() = default;
    ExclusiveResource(const T &value) : mResource(value) {}

    /* Handle to the underlying resource */
    class Handle
    {
    public:
        T &getResource()
        {
            return mResource;
        }

    private:
        friend ExclusiveResource;

        Handle(std::mutex &resourceMutex, T &resource) :
            mLocker(resourceMutex), mResource(resource) {}

        std::lock_guard<std::mutex> mLocker;
        T &mResource;
    };

    using HandlePtr = std::unique_ptr<Handle>;

    /* @return a handle to use the resource.
     *
     * The resource is locked during the handle life time.
     */
    HandlePtr acquireResource()
    {
        return std::unique_ptr<Handle>(new Handle(mResourceMutex, mResource));
    }

private:
    ExclusiveResource(const ExclusiveResource&) = delete;
    ExclusiveResource& operator=(const ExclusiveResource&) = delete;

    std::mutex mResourceMutex;
    T mResource;
};

}
}

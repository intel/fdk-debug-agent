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

#include "cAVS/DspFw/Probe.hpp"
#include "cAVS/Prober.hpp"

#include "Util/AssertAlways.hpp"
#include "Util/PointerHelper.hpp"
#include "Util/Stream.hpp"
#include "Util/Exception.hpp"
#include "Util/Buffer.hpp"
#include "Util/BlockingQueue.hpp"

#include <future>
#include <memory>
#include <map>

namespace debug_agent
{
namespace cavs
{

/** Active object that performs probe packets extraction from an input stream, and then dispatches
 * them to the matching queue */
class ProbeExtractor
{
public:
    using ProbePointMap = std::map<dsp_fw::ProbePointId, ProbeId>;
    using Exception = util::Exception<ProbeExtractor>;
    using BlockingPacketQueue = util::BlockingQueue<util::Buffer>;
    using BlockingExtractionQueues = std::vector<BlockingPacketQueue>;

    /** The constructor starts the probe extractor thread
     *
     * @param[in] extractionQueues to push extracted and demultiplexed probe streams.
     * @param[in] probePointMap A <probe point id, probe index> map used to deduce probe index
     *                          from probe point id.
     * @param[in] inputStream the extraction queues that will receive the packets
     */
    ProbeExtractor(BlockingExtractionQueues &extractionQueues, const ProbePointMap &probePointMap,
                   std::unique_ptr<util::InputStream> inputStream)
        : mExtractionQueues(extractionQueues), mInputStream(std::move(inputStream)),
          mProbePointMap(probePointMap)
    {
        // Clearing the extraction queues at session start, in this way data can still be retrieved
        // after session stop
        for (auto &queue : mExtractionQueues) {
            queue.clear();
        }

        // Starting extractor thread
        mExtractionResult = std::async(std::launch::async, &ProbeExtractor::extract, this);
    }

    ~ProbeExtractor() { stop(); }

    /** Stop the extractor thread */
    void stop() { mInputStream->close(); }

private:
    ProbeExtractor(const ProbeExtractor &) = delete;
    ProbeExtractor &operator=(const ProbeExtractor &) = delete;

    void extract()
    {
        util::ByteStreamReader byteReader(*mInputStream);
        try {
            while (true) {

                // reading one packet
                dsp_fw::Packet packet;
                byteReader.read(packet);

                // finding the probe index that matches the probe point id
                auto it = mProbePointMap.find(packet.probePointId);
                if (it == mProbePointMap.end()) {
                    throw Exception("Packet with unknown probe point id: " +
                                    packet.probePointId.toString());
                }

                // Checking that the probe index is in a valid range
                ProbeId probeId(it->second);
                if (probeId.getValue() >= mExtractionQueues.size()) {
                    throw Exception("Packet with wrong probe id: " +
                                    std::to_string(probeId.getValue()));
                }

                // Writing the packet to a buffer
                auto buffer = std::make_unique<util::Buffer>();
                util::MemoryOutputStream outputStream(*buffer);
                util::ByteStreamWriter writer(outputStream);

                // Writing packet to the stream but with an uint32_t checksum to ensure
                // compatibility with the fdk tool
                // @todo remove this specificity when the fdk tools supports 64bits checksum
                packet.toStream<uint32_t>(writer);

                // Enqueueing the buffer into the right queue
                if (!mExtractionQueues[probeId.getValue()].add(std::move(buffer))) {
                    std::cerr << "Warning: extraction packet dropped." << std::endl;
                }
            }
        } catch (util::ByteStreamReader::EOSException &) {
            return;
        } catch (std::exception &e) {
            std::string message = "Aborting probe extraction due to: ";
            Exception ex(message + e.what());
            std::cerr << ex.what() << std::endl;
            throw ex;
        }
    }

    BlockingExtractionQueues &mExtractionQueues;

    /** Probe extraction is performed by an input stream that read from the probe device. */
    std::unique_ptr<util::InputStream> mInputStream;

    ProbePointMap mProbePointMap;

    std::future<void> mExtractionResult;
};
}
}

/*
 * Copyright (c) 2016, Intel Corporation
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

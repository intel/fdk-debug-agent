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
#include "Core/DebugResources.hpp"
#include "cAVS/FirmwareTypes.hpp"
#include "Util/StringHelper.hpp"
#include "Util/Uuid.hpp"
#include <sstream>
#include <vector>
#include <iomanip>

using namespace debug_agent::rest;
using namespace debug_agent::cavs;
using namespace debug_agent::util;

namespace debug_agent
{
namespace core
{

static const std::string ContentTypeHtml("text/html");

/** Html helper class
 *
 * It provides very basic html features, such as title, paragraph and array
 */
class HtmlHelper
{
public:
    void title(const std::string &title)
    {
        mContent << "<h2>" + title + "</h2>\n";
    }

    void paragraph(const std::string &text)
    {
        mContent << "<p>" + text + "</p>\n";
    }

    void beginTable(const std::vector<std::string> &columnNames)
    {
        mContent << "<table border='1'>\n";
        beginRow();
        for (auto &columnName : columnNames) {
            mContent << "<td nowrap><b>" << columnName << "</b></td>\n";
        }
        endRow();
    }

    void endTable()
    {
        mContent << "</table>\n";
    }

    void beginRow()
    {
        mContent << "<tr>\n";
    }

    void endRow()
    {
        mContent << "</tr>\n";
    }

    void beginCell()
    {
        mContent << "<td nowrap>\n";
    }

    void endCell()
    {
        mContent << "</td>\n";
    }

    template <typename T>
    void cell(const T &value)
    {
        mContent << "<td nowrap>" << value << "</td>\n";
    }

    std::string getHtmlContent() const
    {
        return "<html><body>\n" + mContent.str() + "<body><html>\n";
    }

private:
    std::stringstream mContent;
};

/** Helper method to convert an hash array into string */
std::string hashToString(const uint8_t hash[DEFAULT_HASH_SHA256_LEN])
{
    std::stringstream stream;

    /* Common settings*/
    stream << std::setfill('0') << std::hex << std::uppercase;

    /* Each byte */
    for (std::size_t i = 0; i < DEFAULT_HASH_SHA256_LEN; i++) {
        stream << std::setw(2) << static_cast<uint32_t>(hash[i]);
    }

    return stream.str();
}

void ModuleListDebugResource::handleGet(const Request &request, Response &response)
{
    /* Segment count per module entry, as defined in firmware structures */
    static const std::size_t segmentCount = 3;

    /* Retrieving module entries, doesn't throw exception */
    const std::vector<ModuleEntry> &entries = mSystem.getModuleEntries();

    HtmlHelper html;
    html.title("Module type list");
    html.paragraph("Module count: " + std::to_string(entries.size()));

    /* Module list column names */
    std::vector<std::string> columns = {
        "module_id",
        "name",
        "uuid",
        "struct_id",
        "type",
        "hash",
        "entry_point",
        "cfg_offset",
        "cfg_count",
        "affinity_mask",
        "intance_max_count",
        "instance_stack_size"
    };

    /* Adding segment column names */
    for (std::size_t i = 0; i < segmentCount; ++i) {
        const std::string segmentName("segments[" + std::to_string(i) + "]");
        columns.push_back(segmentName + ".flags");
        columns.push_back(segmentName + ".v_base_addr");
        columns.push_back(segmentName + ".file_offset");
    }

    html.beginTable(columns);

    std::size_t moduleId = 0;
    for (auto &entry : entries)
    {
        html.beginRow();

        html.cell(moduleId);

        std::string name(
            util::StringHelper::getStringFromFixedSizeArray(entry.name, sizeof(entry.name)));
        html.cell(name);

        util::Uuid uuid;
        uuid.fromOtherUuidType(entry.uuid);
        html.cell(uuid.toString());

        html.cell(entry.struct_id);
        html.cell(entry.type.ul);
        html.cell(hashToString(entry.hash));
        html.cell(entry.entry_point);
        html.cell(entry.cfg_offset);
        html.cell(entry.cfg_count);
        html.cell(entry.affinity_mask);
        html.cell(entry.instance_max_count);
        html.cell(entry.instance_stack_size);

        for (std::size_t i = 0; i < segmentCount; ++i) {
            html.cell(entry.segments[i].flags.ul);
            html.cell(entry.segments[i].v_base_addr);
            html.cell(entry.segments[i].file_offset);
        }

        html.endRow();

        moduleId++;
    }

    html.endTable();

    std::ostream &out = response.send(ContentTypeHtml);
    out << html.getHtmlContent();
}


}
}

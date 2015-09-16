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

#include <string>
#include <sstream>
#include <vector>

namespace debug_agent
{
namespace core
{

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

}
}

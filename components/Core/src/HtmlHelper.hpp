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
    void title(const std::string &title) { mContent << "<h2>" + title + "</h2>\n"; }

    void paragraph(const std::string &text) { mContent << "<p>" + text + "</p>\n"; }

    void beginTable(const std::vector<std::string> &columnNames)
    {
        mContent << "<table border='1'>\n";
        beginRow();
        for (auto &columnName : columnNames) {
            mContent << "<td nowrap><b>" << columnName << "</b></td>\n";
        }
        endRow();
    }

    void endTable() { mContent << "</table>\n"; }

    void beginRow() { mContent << "<tr>\n"; }

    void endRow() { mContent << "</tr>\n"; }

    void beginCell() { mContent << "<td nowrap>\n"; }

    void endCell() { mContent << "</td>\n"; }

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

///////
///
///  Copyright (C) 2026  sonoransun
///
///  Permission is hereby granted, free of charge, to any person obtaining a copy
///  of this software and associated documentation files (the "Software"), to deal
///  in the Software without restriction, including without limitation the rights
///  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
///  copies of the Software, and to permit persons to whom the Software is
///  furnished to do so, subject to the following conditions:
///
///  The above copyright notice and this permission notice shall be included in all
///  copies or substantial portions of the Software.
///
///  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
///  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
///  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
///  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
///  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
///  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
///  SOFTWARE.
///
///////


#include <XmlWriter.h>


string
XmlWriter::result ()
{
    return std::move(buffer_);
}


void
XmlWriter::declaration ()
{
    buffer_ += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
}


void
XmlWriter::beginElement (const string & name)
{
    closeOpenTag();
    buffer_ += "<";
    buffer_ += name;
    inOpenTag_ = true;
}


void
XmlWriter::beginElement (const string & name, const string & ns)
{
    closeOpenTag();
    buffer_ += "<";
    buffer_ += name;
    buffer_ += " xmlns=\"";
    buffer_ += escape(ns);
    buffer_ += "\"";
    inOpenTag_ = true;
}


void
XmlWriter::endElement (const string & name)
{
    closeOpenTag();
    buffer_ += "</";
    buffer_ += name;
    buffer_ += ">";
}


void
XmlWriter::attribute (const string & name, const string & value)
{
    if (inOpenTag_) {
        buffer_ += " ";
        buffer_ += name;
        buffer_ += "=\"";
        buffer_ += escape(value);
        buffer_ += "\"";
    }
}


void
XmlWriter::text (const string & content)
{
    closeOpenTag();
    buffer_ += escape(content);
}


void
XmlWriter::selfClosing (const string & name)
{
    closeOpenTag();
    buffer_ += "<";
    buffer_ += name;
    buffer_ += "/>";
}


void
XmlWriter::rawXml (const string & xml)
{
    closeOpenTag();
    buffer_ += xml;
}


void
XmlWriter::simpleElement (const string & name, const string & content)
{
    closeOpenTag();
    buffer_ += "<";
    buffer_ += name;
    buffer_ += ">";
    buffer_ += escape(content);
    buffer_ += "</";
    buffer_ += name;
    buffer_ += ">";
}


void
XmlWriter::closeOpenTag ()
{
    if (inOpenTag_) {
        buffer_ += ">";
        inOpenTag_ = false;
    }
}


string
XmlWriter::escape (const string & s)
{
    string out;
    out.reserve(s.length());

    for (char c : s)
    {
        switch (c)
        {
            case '&':   out += "&amp;";   break;
            case '<':   out += "&lt;";    break;
            case '>':   out += "&gt;";    break;
            case '"':   out += "&quot;";  break;
            default:    out += c;         break;
        }
    }

    return out;
}

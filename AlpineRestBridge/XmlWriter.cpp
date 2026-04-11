/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <XmlWriter.h>


string
XmlWriter::result()
{
    return std::move(buffer_);
}


void
XmlWriter::declaration()
{
    buffer_ += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
}


void
XmlWriter::beginElement(const string & name)
{
    closeOpenTag();
    buffer_ += "<";
    buffer_ += name;
    inOpenTag_ = true;
}


void
XmlWriter::beginElement(const string & name, const string & ns)
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
XmlWriter::endElement(const string & name)
{
    closeOpenTag();
    buffer_ += "</";
    buffer_ += name;
    buffer_ += ">";
}


void
XmlWriter::attribute(const string & name, const string & value)
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
XmlWriter::text(const string & content)
{
    closeOpenTag();
    buffer_ += escape(content);
}


void
XmlWriter::selfClosing(const string & name)
{
    closeOpenTag();
    buffer_ += "<";
    buffer_ += name;
    buffer_ += "/>";
}


void
XmlWriter::rawXml(const string & xml)
{
    closeOpenTag();
    buffer_ += xml;
}


void
XmlWriter::simpleElement(const string & name, const string & content)
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
XmlWriter::closeOpenTag()
{
    if (inOpenTag_) {
        buffer_ += ">";
        inOpenTag_ = false;
    }
}


string
XmlWriter::escape(const string & s)
{
    string out;
    out.reserve(s.length());

    for (char c : s) {
        switch (c) {
        case '&':
            out += "&amp;";
            break;
        case '<':
            out += "&lt;";
            break;
        case '>':
            out += "&gt;";
            break;
        case '"':
            out += "&quot;";
            break;
        default:
            out += c;
            break;
        }
    }

    return out;
}

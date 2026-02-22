/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>


class XmlWriter
{
  public:

    string  result ();

    void  declaration ();

    void  beginElement (const string & name);
    void  beginElement (const string & name, const string & ns);
    void  endElement (const string & name);

    void  attribute (const string & name, const string & value);
    void  text (const string & content);
    void  selfClosing (const string & name);
    void  rawXml (const string & xml);

    void  simpleElement (const string & name, const string & content);


  private:

    static string  escape (const string & s);

    void  closeOpenTag ();

    string  buffer_;
    bool    inOpenTag_ = false;

};

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


#pragma once
#include <Common.h>
#include <Mutex.h>
#include <unordered_map>


class ContentStore
{
  public:

    struct MediaItem {
        string  id;
        string  path;
        string  fileName;
        string  title;
        string  mimeType;
        ulong   fileSize;
    };

    bool   initialize (const string & mediaDirectory);
    void   rescan ();

    bool   getItem (const string & id, MediaItem & item);
    void   getAllItems (vector<MediaItem> & items);
    ulong  getItemCount ();
    ulong  getSystemUpdateId ();


  private:

    void  scanDirectory (const string & dir);

    static const std::unordered_map<string, string>  mimeTypes_s;

    string                                   mediaDirectory_;
    vector<MediaItem>                        items_;
    std::unordered_map<string, ulong>        idIndex_;
    ulong                                    nextId_ = 1;
    ulong                                    systemUpdateId_ = 1;
    Mutex                                    lock_;

};

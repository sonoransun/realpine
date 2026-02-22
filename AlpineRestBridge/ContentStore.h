/// Copyright (C) 2026 sonoransun — see LICENCE.txt


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

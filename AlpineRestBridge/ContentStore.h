/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <Platform.h>
#include <ReadWriteSem.h>
#include <AutoThread.h>
#include <unordered_map>
#include <atomic>
#include <memory>


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

    using ItemMap = std::unordered_map<string, MediaItem>;

    bool   initialize (const string & mediaDirectory);
    void   shutdown ();
    void   rescan ();

    bool   getItem (const string & id, MediaItem & item);
    const ItemMap &  getAllItemsMap ();
    void   getAllItems (vector<MediaItem> & items);
    ulong  getItemCount ();
    ulong  getSystemUpdateId ();


  private:

    void  scanDirectory (const string & dir);
    void  indexFile (const string & fullPath);
    void  removeFile (const string & fullPath);

    static const std::unordered_map<string, string>  mimeTypes_s;

    string                                   mediaDirectory_;
    ItemMap                                  items_;
    std::unordered_map<string, string>       idToPath_;
    ulong                                    nextId_ = 1;
    ulong                                    systemUpdateId_ = 1;
    ReadWriteSem                             lock_;


    // --- Filesystem watcher ---

    class WatcherThread : public AutoThread
    {
      public:
        WatcherThread (ContentStore & store) : store_(store) {}
        void threadMain () override;

      private:
        ContentStore &  store_;
    };

    std::unique_ptr<WatcherThread>  watcher_;
    std::atomic<bool>        watcherStop_{false};

    void  startWatcher ();
    void  stopWatcher ();

#if defined(ALPINE_PLATFORM_DARWIN)
    int   kqueueFd_ = -1;
    void  watcherLoopKqueue ();
#elif defined(ALPINE_PLATFORM_LINUX)
    int   inotifyFd_ = -1;
    void  watcherLoopInotify ();
#endif

    void  watcherLoopFallback ();

    static constexpr int  fullScanIntervalSec_ = 300;
};

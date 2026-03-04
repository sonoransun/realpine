/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <AutoThread.h>
#include <ReadWriteSem.h>
#include <memory>

class VfsNode;


class AlpineFuse
{
  public:

    AlpineFuse ()  = default;
    ~AlpineFuse () = default;


    static bool  initialize (const string &  mountPoint,
                             ulong           cacheTtlSeconds    = 60,
                             ulong           feedbackThreshold  = 5);

    static bool  run ();

    static bool  shutdown ();

    static bool  isRunning ();

    static const string &  getMountPoint ();

    static VfsNode *  rootNode ();

    static ulong  feedbackThreshold ();


  private:

    class FuseThread : public AutoThread
    {
      public:
        void  threadMain () override;
    };

    static void  buildRootTree ();

    static bool  prepareMountPoint ();


    static std::unique_ptr<VfsNode>       root_s;
    static std::unique_ptr<FuseThread>    fuseThread_s;
    static string                         mountPoint_s;
    static ulong                          cacheTtlSeconds_s;
    static ulong                          feedbackThreshold_s;
    static bool                           running_s;
    static ReadWriteSem                   dataLock_s;
};

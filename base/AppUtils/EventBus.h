/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <functional>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <cstdint>


enum class t_Event : uint {
    PeerDiscovered,
    PeerDisconnected,
    QueryCompleted,
    QueryProgress,
    GroupChanged
};


class EventBus
{
  public:

    using t_SubscriberId = uint64_t;
    using t_Callback     = std::function<void(t_Event, const string&)>;


    static t_SubscriberId  subscribe (t_Event     event,
                                      t_Callback  callback);

    static void  unsubscribe (t_SubscriberId subscriberId);

    static void  publish (t_Event         event,
                          const string &  data = ""s);

    static void  clear ();


  private:

    struct Subscriber {
        t_SubscriberId  id;
        t_Callback      callback;
    };

    static std::mutex                                             mutex_s;
    static std::unordered_map<t_Event, vector<Subscriber>>        subscribers_s;
    static t_SubscriberId                                         nextId_s;

};

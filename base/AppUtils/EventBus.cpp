/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include "EventBus.h"
#include "Log.h"


std::mutex EventBus::mutex_s;
std::unordered_map<t_Event, vector<EventBus::Subscriber>> EventBus::subscribers_s;
EventBus::t_SubscriberId EventBus::nextId_s = 0;


EventBus::t_SubscriberId
EventBus::subscribe(t_Event event, t_Callback callback)
{
    std::lock_guard<std::mutex> lock(mutex_s);

    auto id = ++nextId_s;
    subscribers_s[event].push_back({id, std::move(callback)});

    return id;
}


void
EventBus::unsubscribe(t_SubscriberId subscriberId)
{
    std::lock_guard<std::mutex> lock(mutex_s);

    for (auto & [event, subs] : subscribers_s) {
        std::erase_if(subs, [subscriberId](const Subscriber & s) { return s.id == subscriberId; });
    }
}


void
EventBus::publish(t_Event event, const string & data)
{
    vector<t_Callback> callbacks;

    {
        std::lock_guard<std::mutex> lock(mutex_s);

        auto it = subscribers_s.find(event);
        if (it == subscribers_s.end())
            return;

        for (const auto & sub : it->second)
            callbacks.push_back(sub.callback);
    }

    for (const auto & cb : callbacks) {
        try {
            cb(event, data);
        } catch (...) {
            Log::Error("EventBus: exception in subscriber callback"s);
        }
    }
}


void
EventBus::clear()
{
    std::lock_guard<std::mutex> lock(mutex_s);
    subscribers_s.clear();
    nextId_s = 0;
}

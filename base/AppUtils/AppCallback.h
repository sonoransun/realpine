/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <functional>


template <class ArgumentType>
class AppCallbackBase
{
  public:
    AppCallbackBase()
        : func_()
    {}

    explicit AppCallbackBase(std::function<void(ArgumentType)> func)
        : func_(std::move(func))
    {}


    // Invoke callback
    //
    void
    operator()(ArgumentType callbackArg) const
    {
        func_(callbackArg);
    }


    explicit
    operator bool() const
    {
        return static_cast<bool>(func_);
    }


  private:
    std::function<void(ArgumentType)> func_;
};


class AppCallback
{
  public:
    // Create regular non const member function callback
    //
    template <class Receiver, class ReturnType, class ClassType, class ArgumentType>
    static AppCallbackBase<ArgumentType>
    createCallback(Receiver & receiver, ReturnType (ClassType::*const & callback)(ArgumentType))
    {
        auto memFn = callback;
        auto * recv = &receiver;
        return AppCallbackBase<ArgumentType>([recv, memFn](ArgumentType arg) { (recv->*memFn)(arg); });
    }


    // Create member function callback for constant object
    //
    template <class Receiver, class ReturnType, class ClassType, class ArgumentType>
    static AppCallbackBase<ArgumentType>
    createCallback(Receiver & receiver, ReturnType (ClassType::*const & callback)(ArgumentType) const)
    {
        auto memFn = callback;
        auto * recv = &receiver;
        return AppCallbackBase<ArgumentType>([recv, memFn](ArgumentType arg) { (recv->*memFn)(arg); });
    }


    // Create callback for non member function (static/non member)
    //
    template <class ReturnType, class ArgumentType>
    static AppCallbackBase<ArgumentType>
    createCallback(ReturnType (*callback)(ArgumentType))
    {
        return AppCallbackBase<ArgumentType>([callback](ArgumentType arg) { callback(arg); });
    }
};

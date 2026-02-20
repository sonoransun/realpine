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
#include <functional>


template <class ArgumentType>
class AppCallbackBase
{
  public:

    AppCallbackBase () : func_() {}

    explicit AppCallbackBase (std::function<void(ArgumentType)>  func) :
        func_(std::move(func))
    {}


    // Invoke callback
    //
    void operator()(ArgumentType  callbackArg) const
    {
        func_(callbackArg);
    }


    explicit operator bool () const
    {
        return static_cast<bool>(func_);
    }


  private:

    std::function<void(ArgumentType)>  func_;
};



class AppCallback
{
  public:


    // Create regular non const member function callback
    //
    template <class Receiver, class ReturnType, class ClassType, class ArgumentType>
    static AppCallbackBase<ArgumentType>
    createCallback (Receiver &  receiver,
                    ReturnType (ClassType::* const &  callback)(ArgumentType))
    {
        auto  memFn = callback;
        auto *  recv = &receiver;
        return AppCallbackBase<ArgumentType>(
            [recv, memFn](ArgumentType arg) { (recv->*memFn)(arg); }
        );
    }



    // Create member function callback for constant object
    //
    template <class Receiver, class ReturnType, class ClassType, class ArgumentType>
    static AppCallbackBase<ArgumentType>
    createCallback (Receiver &  receiver,
                    ReturnType (ClassType::* const &  callback)(ArgumentType) const)
    {
        auto  memFn = callback;
        auto *  recv = &receiver;
        return AppCallbackBase<ArgumentType>(
            [recv, memFn](ArgumentType arg) { (recv->*memFn)(arg); }
        );
    }



    // Create callback for non member function (static/non member)
    //
    template <class ReturnType, class ArgumentType>
    static AppCallbackBase<ArgumentType>
    createCallback (ReturnType (*callback)(ArgumentType))
    {
        return AppCallbackBase<ArgumentType>(
            [callback](ArgumentType arg) { callback(arg); }
        );
    }

};




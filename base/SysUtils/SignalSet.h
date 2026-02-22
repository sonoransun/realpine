/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <Platform.h>


// Highest signal value we accept.
//
constexpr int SignalMax = 31;


class SignalSet
{
  public:

    SignalSet ();
    ~SignalSet () = default;


    bool clear ();

    bool fill ();

    bool add (int sigNumber);

    bool remove (int sigNumber);

    bool getSignalSet (sigset_t & signalSet);

    static bool signalAsString (int       sigNumber,
                                string &  sigString);


  private:

    sigset_t     signalSet_;

};


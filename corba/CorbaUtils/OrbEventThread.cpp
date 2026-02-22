/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <OrbEventThread.h>
#include <OrbUtils.h>


// Ctor defaulted in header


OrbEventThread::~OrbEventThread () = default;



void 
OrbEventThread::threadMain ()
{
    OrbUtils::runEventLoop ();
}


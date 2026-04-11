/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineServiceThread.h>
#include <AlpineStack.h>
#include <Log.h>
#include <StringUtils.h>


// Ctor defaulted in header


AlpineServiceThread::~AlpineServiceThread() = default;


void
AlpineServiceThread::threadMain()
{
    // Let the AlpineStack perform processing as desired.  This should never return
    // until application termination.
    //
    AlpineStack::processEvents();
}

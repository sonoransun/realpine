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


#include <Connection.h>
#include <PeerNode.h>
#include <Log.h>


unsigned long     Connection::globalConnectionCount_s = 0;


Connection::Connection (PeerNode * homeNode)
{
#ifdef VERY_VERBOSE
    Log::Debug ("Connection::Connection invoked.");
#endif

    destEndpoint_ = 0;
    homeNode_ = homeNode;

    dataLoss_ = 0;
    msgLoss_ = 0;

    globalConnectionCount_s++;
}


Connection::~Connection ()
{
#ifdef VERY_VERBOSE
    Log::Debug ("Connection::~Connection invoked.");
#endif

    if (destEndpoint_) {
        destEndpoint_->closeConnection ();
    }

    globalConnectionCount_s--;
}


unsigned long  
Connection::getNodeId ()
{
#ifdef VERY_VERBOSE
    Log::Debug ("Connection::getNodeId invoked.");
#endif

    return homeNode_->getId();
}


bool  
Connection::tie (Connection * destEndpoint)
{
#ifdef VERY_VERBOSE
    Log::Debug ("Connection::tie invoked.");
#endif

    if (destEndpoint_) {
        // Duplicate tie request.
        Log::Error ("Duplicate tie request in Connection::tie!");
        return false;
    }
    destEndpoint_ = destEndpoint;
    destEndpoint->destEndpoint_ = this;
 
    return true;
}


void
Connection::closeConnection ()
{
#ifdef VERY_VERBOSE
    Log::Debug ("Connection::closeConnection invoked.");
#endif

    homeNode_->closeConnection (destEndpoint_->getNodeId());
}


bool
Connection::sendMsg (const Message * msg)
{
#ifdef VERY_VERBOSE
    Log::Debug ("Connection::sendMsg invoked.");
#endif

    destEndpoint_->processMsg (msg);

    return true;
}


bool  
Connection::processMsg (const Message * msg)
{
#ifdef VERY_VERBOSE
    Log::Debug ("Connection::processMsg invoked.");
#endif

    if (homeNode_->remainingBandwidth () > (unsigned int)msg->getSize ()) {
        // we have room for this message
        homeNode_->consumeBandwidth (msg->getSize ());
        homeNode_->processMessage (msg);
    }
    else {
        // Overflow!!
        dataLoss_ += msg->getSize ();
        msgLoss_++;
        return;
    }

    return true;
}


long  
Connection::dataLoss ()
{
    return dataLoss_;
}


long  
Connection::msgLoss ()
{
    return msgLoss_;
}


void  
Connection::reset ()
{
#ifdef VERY_VERBOSE
    Log::Debug ("Connection::reset invoked.");
#endif

    dataLoss_ = msgLoss_ = 0;
}

    
unsigned long  
Connection::globalConnectionCount ()
{
    return globalConnectionCount_s;
}


/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <Message.h>
#include <Log.h>


unsigned long      Message::messageCount_s = 0;
unsigned long      Message::dataCount_s = 0;


Message::Message (int size, int ttl, unsigned long id)
  : size_(size), hops_(0), ttl_(ttl), id_(id)
{
#ifdef VERY_VERBOSE
    Log::Debug ("Message::Message (param) invoked.");
#endif

    Message::messageCreated (size);
}


Message::~Message ()
{
#ifdef VERY_VERBOSE
    Log::Debug ("Message::~Message invoked.");
#endif

}


Message::Message (const Message & copy)
{
#ifdef VERY_VERBOSE
    Log::Debug ("Message::Message (copy) invoked.");
#endif

    size_ = copy.size_;
    hops_ = copy.hops_ + 1;
    ttl_ = copy.ttl_ - 1;
    id_ = copy.id_;

    Message::messageCreated (size_);
}


Message & 
Message::operator = (const Message & copy)
{
#ifdef VERY_VERBOSE
    Log::Debug ("Message::operator = invoked.");
#endif

    if (&copy == this) {
        // no self assignment
        return *this;
    }

    size_ = copy.size_;
    hops_ = copy.hops_ + 1;
    ttl_ = copy.ttl_ - 1;
    id_ = copy.id_;

    Message::messageCreated (size_);

    return *this;
}


unsigned long  
Message::getId () const
{
    return id_;
}


int  
Message::getSize () const
{
    return size_;
}


int  
Message::getHops () const
{
    return hops_;
}


int  
Message::getTtl  () const
{
    return ttl_;
}


void
Message::decTtl ()
{
    ttl_--;
}


void   
Message::messageCreated (int size)
{
#ifdef VERY_VERBOSE
    Log::Debug ("Message::messageCreated invoked.");
#endif

    Message::messageCount_s++;
    Message::dataCount_s += size;
}


unsigned long  
Message::globalMessageCount ()
{
    return Message::messageCount_s;
}


unsigned long
Message::globalDataCount ()
{
    return Message::dataCount_s;
}


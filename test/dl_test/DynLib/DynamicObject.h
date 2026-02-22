/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <DynamicBase.h>


// Entry point for dynamic test library
//
extern "C" {

void *  createDynamicObject ();

}


class DynamicObject : public DynamicBase
{
  public:

    DynamicObject (ulong  id);
    virtual ~DynamicObject ();


    virtual void testMethod ();

  private:

    ulong   myId_;

    static ulong   currMethodNum_s;

    friend void *createDynamicObject ();
};


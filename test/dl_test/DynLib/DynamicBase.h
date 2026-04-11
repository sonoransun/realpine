/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>


class DynamicBase
{
  public:
    DynamicBase(){};
    virtual ~DynamicBase(){};

    virtual void testMethod() = 0;
};

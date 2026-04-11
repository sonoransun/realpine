/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <DynamicObject.h>
#include <Log.h>
#include <StringUtils.h>


ulong DynamicObject::currMethodNum_s = 1;


extern "C" {

void *
createDynamicObject()
{
    DynamicObject * retObject;
    retObject = new DynamicObject(DynamicObject::currMethodNum_s++);

    return retObject;
}
}


DynamicObject::DynamicObject(ulong id)
    : myId_(id)
{}


DynamicObject::~DynamicObject() = default;


void
DynamicObject::testMethod()
{
    Log::Info("DynamicObject::testMethod invoked on Object ID: "s + std::to_string(myId_));
}

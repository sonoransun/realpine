/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <DynamicLoader.h>
#include <ApplCore.h>
#include <Configuration.h>
#include <Log.h>
#include <StringUtils.h>
#include <DynamicBase.h>
#include <vector>
#include <stdlib.h>



int 
main (int argc, char *argv[])
{
    // Initialize application core
    //
    bool status;
    status = ApplCore::initialize (argc, argv);

    if (!status) {
        Log::Error ("Initialization of Application Core failed!  Exiting...");
        return 1;
    }


    // Initialize configuration
    //
    Log::Debug ("Initializing configuration...");
 
    ConfigData::t_ConfigElementList * configElements;
    ConfigData::t_ConfigElement *  currElement;

    // Dynamic library name
    configElements = new ConfigData::t_ConfigElementList;
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Library Name";
    currElement->argOptionName = "libraryName";
    currElement->envOptionName = "LIB_NAME";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = true;
    configElements->push_back (currElement);

    // Entry point method
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Entry Function";
    currElement->argOptionName = "entryFunction";
    currElement->envOptionName = "ENTRY_FUNC";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = true;
    configElements->push_back (currElement);

    // Number of objects to create
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Num Objects";
    currElement->argOptionName = "numObjects";
    currElement->envOptionName = "NUM_OBJECTS";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = true;
    configElements->push_back (currElement);

    status = Configuration::initialize (argc,
                                        argv,
                                        *configElements,
                                        "dynamicLoaderTest.cfg");

    if (!status) {
        Log::Error ("Initialization of application configuration failed!  Exiting...");
        return 2;
    }


    // Get configuration settings
    //
    Log::Debug ("Loading configuration settings...");
 
    string  libName;
    string  functionName;
    string  numObjectsStr;
    int     numObjects;

    Configuration::getValue ("Library Name", libName);
    Configuration::getValue ("Entry Function", functionName);
    Configuration::getValue ("Num Objects", numObjectsStr);

    numObjects = atoi (numObjectsStr.c_str());

    Log::Info ("Starting dynamic loader test."s +
               "\n Library Name: "s + libName +
               "\n Entry Function: "s + functionName +
               "\n Num Objects: "s + numObjectsStr +
               "\n");


    DynamicLoader  loader;
    status = loader.load (libName);

    if (!status) {
        Log::Error ("Error loading library!  Exiting...\n");
        return 3;
    }

    void * func;
    status = loader.getFunctionHandle (functionName, func);

    if (!status) {
        Log::Error ("Error retrieving function reference!  Exiting...\n");
        return 4;
    }

    DynamicBase * (*objectCreator)();
    objectCreator = (DynamicBase * (*)())func;

    DynamicBase *  currObject;
    using t_ObjectArray = vector<DynamicBase *>;
    t_ObjectArray  objectArray;

    objectArray.resize (numObjects);

    int i;
    for (i = 0; i < numObjects; i++) {
        currObject = (*objectCreator)();

        if (!currObject) {
            Log::Error ("Error creating dynamic object!  Exiting...\n");
            return 5;
        }

        objectArray[i] = currObject;
    }

    Log::Info ("Created all objects.  Invoking derived handlers...");

    for (i = 0; i < numObjects; i++) {
        objectArray[i]->testMethod ();
    }

    Log::Info ("Test finished.");


    return 0;
}




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



#include <Common.h>

#include <CorbaUtils.h>
#include <OrbUtils.h>
#include <CosNamingUtils.h>

#include <Log.h>
#include <StringUtils.h>

#include <ApplCore.h>
#include <Configuration.h>
#include <TestConfig.h>



int 
main (int argc, char *argv[])
{
    bool status;

    // Initialize application core
    //
    status = ApplCore::initialize (argc, argv);

    if (!status) {
        Log::Error ("Unable to initialize application core.  Exiting.");
        exit (1);
  
 
    // Initialize configuration
    //
    ConfigData::t_ConfigElementList * configElements;

    TestConfig::createConfigElements ();
    TestConfig::getConfigElements (configElements);

    status = Configuration::initialize (argc, 
                                        argv, 
                                        *configElements,
                                        TestConfig::configFile_s);

    if (!status) {
        Log::Error ("Error initializing configuration.  Exiting.");
        return 1;
    }
    // Load configuration settings
    //
    string  testContext;
    string  testBinding;

    status = Configuration::getValue ("Test Context", testContext);

    if (!status) {
        Log::Error ("No TestContext value.  Exiting.");
        return 1;
    }
    status = Configuration::getValue ("Test Binding", testBinding);

    if (!status) {
        Log::Error ("No TestBinding value.  Exiting.");
        return 1;
    }
    Log::Info ("Starting CorbaUtilsTest-"s +
               "\nContext: "s + testContext +
               "\nBinding: "s + testBinding +
               "\n");


    // Initialize ORB
    //
    status = OrbUtils::initialize (argc, argv);

    if (!status) {
        Log::Error ("OrbUtils initialization failed!  Exiting.");
        return 1;
    }
    CORBA::ORB_var  orbVar;
    status = OrbUtils::getOrb (orbVar);

    if (!status) {
        Log::Error ("Getting ORB reference failed!  Exiting.");
        return 1;
    }
    // Initialize CosNamingUtils
    //
    status = CosNamingUtils::initialize (orbVar);

    if (!status) {
        Log::Error ("CosNamingUtils initialization failed!  Exiting.");
        return 1;
    }
    // Perform naming service tests
    //
    Log::Info ("Starting naming service tests...");

    CosNamingUtils  namingUtil;


    // Context test
    Log::Info ("Creating context...");

    status = namingUtil.createContext (testContext);

    if (!status) {
        Log::Error ("Test context creation failed!  Exiting.");
        return 1;
    }
    status = namingUtil.setCurrentContext (testContext);

    if (!status) {
        Log::Error ("Setting test context failed!  Exiting.");
        return 1;
    }
    // Binding test
    Log::Info ("Creating bindings...");

    status = namingUtil.bindingExists (testBinding);

    if (status) {
        Log::Debug ("Test binding exists, removing.");
       
        status = namingUtil.removeBinding (testBinding);

        if (!status) {
            Log::Error ("Removing binding failed!  Exiting.");
            return 1;
    }
    else {
        Log::Debug ("Test binding does not exist, continuing.");
        return false;
    }


    CORBA::Object_ptr  testObject = nullptr;  // non standard nill reference

    status = namingUtil.addBinding (testBinding, testObject);

    if (!status) {
        Log::Error ("Adding test binding failed!  Exiting.");
        return 1;
    }
    CORBA::Object_var  resolvedObject;

    status = namingUtil.resolve (testBinding, resolvedObject);

    if (!status) {
        Log::Error ("Resolving test binding failed!  Exiting.");
        return 1;
    }
    status = namingUtil.updateBinding (testBinding, testObject);

    if (!status) {
        Log::Error ("Updating test binding failed!  Exiting.");
        return 1;
    }
    Log::Info ("All naming service tests completed successfully.");



    Log::Info ("Finished.");


    return 0;
}




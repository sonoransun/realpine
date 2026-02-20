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


#include <CosNamingUtils.h>
#include <Log.h>
#include <StringUtils.h>
#include <ReadLock.h>
#include <WriteLock.h>
#include <vector>



bool                               CosNamingUtils::initialized_s = false;
CORBA::ORB_var                     CosNamingUtils::orb_s;
CosNaming::NamingContext_var       CosNamingUtils::rootContext_s;
TAO_Naming_Client *                CosNamingUtils::namingClient_s = nullptr;
ReadWriteSem                       CosNamingUtils::dataLock_s;



CosNamingUtils::CosNamingUtils ()
{
#ifdef _VERBOSE
    Log::Debug ("CosNamingUtils::CosNamingUtils constructor invoked.");
#endif

    ReadLock  lock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("Creating CosNamingUtils object without initialization!");
        return false;
    }

    rootContext_    = rootContext_s;
    currentContext_ = rootContext_s;

    contextName_    = "/";
}



CosNamingUtils::~CosNamingUtils ()
{
#ifdef _VERBOSE
    Log::Debug ("CosNamingUtils::CosNamingUtils destructor invoked.");
#endif

    // Nothing to do here.  Vars will cleanup.
}


  
bool
CosNamingUtils::initialize (CORBA::ORB_var &  orb)
{
#ifdef _VERBOSE
    Log::Debug ("CosNamingUtils::initialize invoked.");
#endif

    WriteLock  lock(dataLock_s);

    if (initialized_s) {
        Log::Error ("Attempt to reinitialize CosNamingUtils!");
        return false;
    }
    orb_s = orb;
    namingClient_s = new TAO_Naming_Client;

    ExNewEnv;

    ExTry {

        namingClient_s->init (orb.in());

        rootContext_s   = namingClient_s->get_context ();

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {

        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " while initializing Naming Client in CosNamingUtils::initialize.");
        return false;

    }
    ExCatchAny {

        Log::Error ("Caught unexpected exception while initializing "
                             "Naming Client in CosNamingUtils::initialize.");
        return false;

    }
    ExEndTry;
    ExCheck;

    initialized_s = true;


    return true;
}



bool 
CosNamingUtils::getRootContext (CosNaming::NamingContext_var &  context)
{
#ifdef _VERBOSE
    Log::Debug ("CosNamingUtils::getRootContext invoked.");
#endif

    ReadLock  lock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("CosNamingUtils is uninitialized in call to getRootContext!");
        return false;
    }
    context = rootContext_s;


    return true;
}



bool 
CosNamingUtils::getContextList (t_ContextList &  contextList)
{
#ifdef _VERBOSE
    Log::Debug ("CosNamingUtils::getContextList invoked.");
#endif

    // MRP_TEMP not implemented.

    return true;
}



bool 
CosNamingUtils::getBindingList (t_BindingList &  bindingList)
{
#ifdef _VERBOSE
    Log::Debug ("CosNamingUtils::getBindingList invoked.");
#endif

    // MRP_TEMP not implemented.

    return true;
}


 
bool 
CosNamingUtils::setCurrentContext (const string & contextName)
{
#ifdef _VERBOSE
    Log::Debug ("CosNamingUtils::setCurrentContext invoked for name: "s + contextName);
#endif

    // Setup for context location
    //
    bool status; 
    t_ContextList  contextList;

    status = parseContextPath (contextName, contextList);

    if (!status) {
        Log::Error ("Parsing context name into path list failed in"
                             " CosNamingUtils::setCurrentContext.");
        return false;
    }
    CosNaming::Name_var  cosName;
    status = generateCosName (contextList, cosName);

    if (!status) {
        Log::Error ("Generating CosName from name list failed in"
                             " CosNamingUtils::setCurrentContext.");
        return false;
    }
    bool  relativeContext;
    if (contextName[0] == '/') {
        relativeContext = false;
    }
    else {
        relativeContext = true;
    }


    // Obtain reference to this naming context from root or current reference
    //
    CORBA::Object_var  namingObject;

    ExNewEnv;

    ExTry {

        if (relativeContext) {
            namingObject = currentContext_->resolve (cosName.in());
        }
        else {
            namingObject = rootContext_->resolve (cosName.in());
        }
 
        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {

        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " when resolving context name: "s + contextName);
        return false;

    }
    ExCatchAny {

        Log::Debug ("Error resolving context name: "s + contextName);
        return false;

    }
    ExEndTry;
    ExCheck;


    // Narrow to a naming context
    //
    CosNaming::NamingContext_var  newContext;

    ExTry {

        newContext = CosNaming::NamingContext::_narrow (namingObject.in());

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {

        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " when narrowing context binding.  Ignoring.");
        return false;

    }
    ExCatchAny {
        Log::Debug ("Caught exception during narrow of context name: "s + contextName +
                    "  Ignoring.");
        return false;

    }
    ExEndTry;
    ExCheck;

    
    // Update our current context information.
    //    
    currentContext_ = newContext;
    contextPath_    = contextList;
    contextName_    = contextName;

#ifdef _VERBOSE
    Log::Debug ("New current context set to: "s + contextName_);
#endif
    

    return true;
}

 

bool 
CosNamingUtils::getCurrentContext (string & contextName)
{
#ifdef _VERBOSE
    Log::Debug ("CosNamingUtils::getCurrentContext invoked.");
#endif

    contextName = contextName_;


    return true;
}



bool 
CosNamingUtils::createContext (const string & contextName)
{
#ifdef _VERBOSE
    Log::Debug ("CosNamingUtils::createContext invoked for name: "s + contextName);
#endif

    // Setup for context location
    //
    bool status; 
    t_ContextList  contextList;

    status = parseContextPath (contextName, contextList);

    if (!status) {
        Log::Error ("Parsing context name into path list failed in"
                             " CosNamingUtils::createContext.");
        return false;
    }
    // We are going to create a context, so all contexts up to the last
    // one should exist, however, we will perform a brute creation, and
    // create all contexts required for the last context.
    //
    t_CosNameList  cosNameList;
    status = generateCosNameList (contextList, cosNameList);

    if (!status) {
        Log::Error ("Generating CosNameList from name list failed in"
                             " CosNamingUtils::setCurrentContext.");
        return false;
    }
    bool  relativeContext;
    if (contextName[0] == '/') {
        relativeContext = false;
    }
    else {
        relativeContext = true;
    }


    // Iterate through each level of contexts and create if nescessary
    //
    CosNaming::Name_var           currentName;
    CORBA::Object_var             namingObject;
    CosNaming::NamingContext_var  newContext;

    ExNewEnv;

    for (auto& item : cosNameList) {

        currentName = item;
        bool  exists;

        ExTry {

            if (relativeContext) {
                namingObject = currentContext_->resolve (currentName.in());
            }
            else {
                namingObject = rootContext_->resolve (currentName.in());
            }
 
            ExTryCheck;

            exists = true;
        }
        ExCatch (CORBA::Exception, ex) {

            Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                        " when resolving context.  Ignoring.");
            exists = false;
        }
        ExCatchAny {
            // This current context does not currently exist.  Create it.
            //
            exists = false;
        }
        ExEndTry;
        ExCheck;

   
        if (exists) {
 
            // Narrow to a naming context to verify type of binding
            //
            ExTry {
    
                newContext = CosNaming::NamingContext::_narrow (namingObject.in());
    
                ExTryCheck;
            }
            ExCatch (CORBA::Exception, ex) {
        
                Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                            " when binding within context path: "s + contextName);
                return false;

            }
            ExCatchAny {

                Log::Debug ("Caught exception narrowing within context path: "s + contextName +
                            "  Ignoring.");
                return false;

            }
            ExEndTry;
            ExCheck;

            // This level of the target context exists and is ready, continue to next...
            //
            continue;
        }
        else {

            // This context does not exist.  Create new context.
            //
            ExTry {

                if (relativeContext) {
                    newContext = currentContext_->bind_new_context (currentName.in());
                }
                else {
                    newContext = rootContext_->bind_new_context (currentName.in());
                }
 
                ExTryCheck;
            }
            ExCatch (CORBA::Exception, ex) {

                Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                            " when binding context within context path: "s + contextName);
                return false;

            }
            ExCatchAny {

                Log::Debug ("Bind new context within context path: "s + contextName +
                            " failed.");
                return false;

            }
            ExEndTry;
            ExCheck;

            // This level of the target context exists and is ready, continue to next...
            //
            continue;
        }
    }


    return true;
}



bool  
CosNamingUtils::bindingExists (const string &  name)
{
#ifdef _VERBOSE
    Log::Debug ("CosNamingUtils::bindingExists invoked for name: "s + name);
#endif

    // Setup for binding location
    //
    bool status; 
    t_ContextList  contextList;

    status = parseContextPath (name, contextList);

    if (!status) {
        Log::Error ("Parsing binding name into path list failed in"
                             " CosNamingUtils::bindingExists.");
        return false;
    }
    CosNaming::Name_var  cosName;
    status = generateCosName (contextList, cosName);

    if (!status) {
        Log::Error ("Generating CosName from name list failed in"
                             " CosNamingUtils::bindingExists.");
        return false;
    }
    bool  relativeContext;
    if (name[0] == '/') {
        relativeContext = false;
    }
    else {
        relativeContext = true;
    }


    // Obtain reference to this binding from root or current reference
    //
    CORBA::Object_var  boundObject;

    ExNewEnv;

    ExTry {

        if (relativeContext) {
            boundObject = currentContext_->resolve (cosName.in());
        }
        else {
            boundObject = rootContext_->resolve (cosName.in());
        }
 
        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {

        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " during resolve.  Assuming object does not exist.");
        return false;

    }
    ExCatchAny {
        // Does not exist...
        //
        return false;

    }
    ExEndTry;
    ExCheck;


    return true;
}



bool 
CosNamingUtils::addBinding (const string &             name,
                            const CORBA::Object_var &  object)
{
#ifdef _VERBOSE
    Log::Debug ("CosNamingUtils::addBinding invoked for name: "s + name);
#endif

    // Setup for binding location
    //
    bool status; 
    t_ContextList  contextList;

    status = parseContextPath (name, contextList);

    if (!status) {
        Log::Error ("Parsing binding name into path list failed in"
                             " CosNamingUtils::addBinding.");
        return false;
    }
    bool  relativeContext;
    if (name[0] == '/') {
        relativeContext = false;
    }
    else {
        relativeContext = true;
    }


    // Since we are binding an object, we need to create all contexts
    // up to the object, then create the object binding itself.
    // The sub context preparation only needs to be done if this object
    // binding name is a compound name of contexts AND binding name.
    //
    if (contextList.size () > 1) {

        string          subContextPath;
        t_ContextList   subContextList (contextList);
        subContextList.pop_back ();  // remove binding name, leave contexts

        status = generateContextPath (subContextList, subContextPath);

        if (!status) {
            Log::Error ("Unable to generate sub context path for binding name: "s +
                        name + " in CosNamingUtils::addBinding.");
            return false;
        }
        // handle absolute path condition
        //
        if (!relativeContext) {
            string temp = subContextPath;
            subContextPath = "/";
            subContextPath += temp;
            return false;
        }


        // Create all sub contexts before creating object binding.
        //
        status = createContext (subContextPath);

        if (!status) {
            Log::Error ("Unable to create sub contexts for binding name: "s +
                        name + " in CosNamingUtils::addBinding.");
            return false;
        }
    }


    // Now we should be ready to create the object binding itself.
    //
    CosNaming::Name_var  cosName;
    status = generateCosName (contextList, cosName);

    if (!status) {
        Log::Error ("Generating CosName from name list failed in"
                             " CosNamingUtils::addBinding.");
        return false;
    }
    // Create binding from root or current context
    //
    ExNewEnv;

    ExTry {

        if (relativeContext) {
            currentContext_->bind (cosName.in(), object.in());
        }
        else {
            rootContext_->bind (cosName.in(), object.in());
        }
 
        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {

        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " when binding object under path: "s + name);
        return false;

    }
    ExCatchAny {

        Log::Error ("Binding object under path: "s + name +
                    " failed in CosNamingUtils::addBinding.");
        return false;

    }
    ExEndTry;
    ExCheck;


    return true;
}



bool  
CosNamingUtils::updateBinding (const string &             name,
                               const CORBA::Object_var &  object)
{
#ifdef _VERBOSE
    Log::Debug ("CosNamingUtils::updateBinding invoked for name: "s + name);
#endif

    // Setup for binding location
    //
    bool status; 
    t_ContextList  contextList;

    status = parseContextPath (name, contextList);

    if (!status) {
        Log::Error ("Parsing binding name into path list failed in"
                             " CosNamingUtils::updateBinding.");
        return false;
    }
    // We should not have to create all sub contexts because this binding
    // already exists (or is supposed to at least).
    //
    CosNaming::Name_var  cosName;
    status = generateCosName (contextList, cosName);

    if (!status) {
        Log::Error ("Generating CosName from name list failed in"
                             " CosNamingUtils::updateBinding.");
        return false;
    }
    bool  relativeContext;
    if (name[0] == '/') {
        relativeContext = false;
    }
    else {
        relativeContext = true;
    }


    // Update binding from root or current context
    //
    ExNewEnv;

    ExTry {

        if (relativeContext) {
            currentContext_->rebind (cosName.in(), object.in ());
        }
        else {
            rootContext_->rebind (cosName.in(), object.in ());
        }
 
        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {

        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " when updating bound object under path: "s + name);
        return false;

    }
    ExCatchAny {

        Log::Error ("Update for bound object under path: "s + name +
                    " failed in CosNamingUtils::updateBinding.");
        return false;

    }
    ExEndTry;
    ExCheck;


    return true;
}



bool 
CosNamingUtils::addContext (const string &                        name,
                            const CosNaming::NamingContext_var &  context)
{
#ifdef _VERBOSE
    Log::Debug ("CosNamingUtils::addContext invoked for name: "s + name);
#endif


    // MRP_TEMP not implemented


    return true;
}



bool 
CosNamingUtils::resolve (const string &       name,
                         CORBA::Object_var &  object)
{
#ifdef _VERBOSE
    Log::Debug ("CosNamingUtils::resolve invoked for name: "s + name);
#endif

    // Setup for binding location
    //
    bool status; 
    t_ContextList  contextList;

    status = parseContextPath (name, contextList);

    if (!status) {
        Log::Error ("Parsing binding name into path list failed in"
                             " CosNamingUtils::resolve.");
        return false;
    }
    // Create CosNaming::Name location for binding
    //
    CosNaming::Name_var  cosName;
    status = generateCosName (contextList, cosName);

    if (!status) {
        Log::Error ("Generating CosName from name list failed in"
                             " CosNamingUtils::resolve.");
        return false;
    }
    bool  relativeContext;
    if (name[0] == '/') {
        relativeContext = false;
    }
    else {
        relativeContext = true;
    }


    // Lookup binding from root or current context
    //
    ExNewEnv;

    ExTry {

        if (relativeContext) {
            object = currentContext_->resolve (cosName.in());
        }
        else {
            object = rootContext_->resolve (cosName.in());
        }
 
        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {

        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " when locating bound object name: "s + name);
        return false;

    }
    ExCatchAny {

        Log::Error ("Locating binding object name: "s + name +
                    " failed in CosNamingUtils::resolve.");
        return false;

    }
    ExEndTry;
    ExCheck;


    return true;
}



bool 
CosNamingUtils::removeBinding (const string & name)
{
#ifdef _VERBOSE
    Log::Debug ("CosNamingUtils::removeBinding invoked for name: "s + name);
#endif

    // Setup for binding location
    //
    bool status; 
    t_ContextList  contextList;

    status = parseContextPath (name, contextList);

    if (!status) {
        Log::Error ("Parsing binding name into path list failed in"
                             " CosNamingUtils::removeBinding.");
        return false;
    }
    // Create CosNaming::Name location for binding
    //
    CosNaming::Name_var  cosName;
    status = generateCosName (contextList, cosName);

    if (!status) {
        Log::Error ("Generating CosName from name list failed in"
                             " CosNamingUtils::removeBinding.");
        return false;
    }
    bool  relativeContext;
    if (name[0] == '/') {
        relativeContext = false;
    }
    else {
        relativeContext = true;
    }


    // Remove binding from root or current context
    //
    ExNewEnv;

    ExTry {

        if (relativeContext) {
            currentContext_->unbind (cosName.in());
        }
        else {
            rootContext_->unbind (cosName.in());
        }
 
        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {

        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " removing binding name: "s + name);
        return false;

    }
    ExCatchAny {
        
        Log::Error ("Remove binding failed for binding name: "s + name +
                    " in CosNamingUtils::removeBinding.");
        return false;

    }
    ExEndTry;
    ExCheck;


    return true;
}



bool 
CosNamingUtils::parseContextPath (const string &   contextPath,
                                  t_ContextList &  contextList)
{
#ifdef _VERBOSE
    Log::Debug ("CosNamingUtils::parseContextPath invoked.");
#endif

    // If the entire command line was passed (erroniously) then split into
    // command and arguments...
    //
    contextList.clear ();

    const char * delimiter = "/";
    char * tokenizeBuffer;
    char * currValue;
    char * nextSubstr  = nullptr;
    const int buffSize = contextPath.size () + 2;
    tokenizeBuffer     = new char[buffSize];
    bool finished      = false;

    strncpy (tokenizeBuffer, contextPath.c_str(), buffSize - 1);

    currValue = strtok_r (tokenizeBuffer, delimiter, &nextSubstr);

    if (!currValue) {
        finished = true;
    }
    else {
        contextList.emplace_back(currValue);
    }

    while (!finished) {

        currValue = strtok_r (nullptr, delimiter, &nextSubstr);

        if (!currValue) {
            finished = true;
            continue;
        }

        contextList.emplace_back(currValue);
    }

    delete [] tokenizeBuffer;


    return true;
}



bool 
CosNamingUtils::generateContextPath (t_ContextList &  contextList,
                                     string &         contextPath)
{
#ifdef _VERBOSE
    Log::Debug ("CosNamingUtils::generateContextPath invoked.");
#endif

    const string  separator ("/");
    contextPath = "";

    if (contextList.empty()) {
        return true;
    }
    auto iter = contextList.begin ();


    // Do not place prepending '/' in generated context as this will
    // confuse relative contexts.  If a path is known to be full, then
    // the '/' can be prepended by the caller.
    //
    contextPath += (*iter);
    iter++;

    for (; iter != contextList.end (); iter++) {
        contextPath += separator;
        contextPath += (*iter);
    }


    return true;
}



bool 
CosNamingUtils::generateCosName (t_ContextList &        contextList,
                                 CosNaming::Name_var &  cosName)
{
#ifdef _VERBOSE
    Log::Debug ("CosNamingUtils::generateCosName invoked.");
#endif

    if (contextList.empty()) {
        return false;
    }
    cosName = new CosNaming::Name;
    cosName->length (contextList.size ());

    int i;
    auto iter = contextList.begin ();

    for (i = 0; iter != contextList.end (); iter++, i++) {
        cosName[i].id  = CORBA::string_dup ((*iter).c_str());
        // not needed > cosName[i].kind = CORBA::string_dup ("");
    }


    return true;
}



bool  
CosNamingUtils::generateCosNameList (t_ContextList &  contextList,
                                     t_CosNameList &  cosNameList)
{
#ifdef _VERBOSE
    Log::Debug ("CosNamingUtils::generateCosNameList invoked.");
#endif

    if (contextList.empty()) {
        return false;
    }
    int arraySize = contextList.size ();

    using t_ContextListArray = vector<t_ContextList>;
    t_ContextListArray  contextListArray (arraySize);

    int currIndex;
    int i;
    // Initialize context list array
    //
    for (currIndex = 0;  currIndex < arraySize;  currIndex++) {

        auto iter = contextList.begin ();
        i    = 0;

        for (; i <= currIndex; iter++, i++) {
            contextListArray[currIndex].push_back (*iter);
        }
    }

    bool status;

    // generate Cos Naming Name list
    //
    CosNaming::Name_var currName;

    for (auto& arrayItem : contextListArray) {

        status = generateCosName (arrayItem, currName);

        if (!status) {
            Log::Error ("Error encountered building cos name list in "
                                 "CosNamingUtils::generateCosNameList.  generateCosName failed.");
            return false;
        }
        cosNameList.push_back (currName);
    }   


    return true;
}



bool 
CosNamingUtils::generateContextPath (CosNaming::Name_var &  cosName,
                                     t_ContextList &        contextList)
{
#ifdef _VERBOSE
    Log::Debug ("CosNamingUtils::generateContextPath invoked.");
#endif

    /// MRP_TEMP not implemented.

    return true;
}



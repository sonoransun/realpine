/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <DynamicLoader.h>
#include <Log.h>
#include <StringUtils.h>


DynamicLoader::DynamicLoader()
{
    libPath_ = "";
    symbolScope_ = t_SymbolScope::Private;
    bindingMethod_ = t_BindingMethod::Now;
    dlFlags_ = RTLD_NOW;
    dlHandle_ = {};
}


DynamicLoader::~DynamicLoader()
{
    if (dlHandle_)
        alpine_dlclose(dlHandle_);
}


bool
DynamicLoader::setSymbolScope(t_SymbolScope scope)
{
#ifdef _VERBOSE
    Log::Debug("DynamicLoader::setSymbolScope invoked.");
#endif

    if ((scope != t_SymbolScope::Private) && (scope != t_SymbolScope::Global)) {
        Log::Error("Invalid symbol scope value passed in call to DynamicLoader::setSymbolScope!");
        return false;
    }

    symbolScope_ = scope;

    return true;
}


DynamicLoader::t_SymbolScope
DynamicLoader::getSymbolScope()
{
    return symbolScope_;
}


bool
DynamicLoader::setBindingMethod(t_BindingMethod method)
{
#ifdef _VERBOSE
    Log::Debug("DynamicLoader::setBindingMethod invoked.");
#endif

    if ((method != t_BindingMethod::Now) && (method != t_BindingMethod::Lazy)) {
        Log::Debug("Invalid binding method value passed in call to DynamicLoader::setBindingMethod!");
        return false;
    }

    bindingMethod_ = method;

    return true;
}


DynamicLoader::t_BindingMethod
DynamicLoader::getBindingMethod()
{
    return bindingMethod_;
}


bool
DynamicLoader::load(const string & libPath)
{
#ifdef _VERBOSE
    Log::Debug("DynamicLoader::load invoked.  Library Path:"s + libPath);
#endif

    libPath_ = libPath;

    dlHandle_ = alpine_dlopen(libPath_.c_str(), dlFlags_);

    if (!dlHandle_) {
        Log::Error("Error opening library: "s + libPath_ +
                   " in call to "
                   "DynamicLoader::load!");
        return false;
    }


    return true;
}


bool
DynamicLoader::getFunctionHandle(const string & functionName, void *& functionHandle)
{
#ifdef _VERBOSE
    Log::Debug("DynamicLoader::getFunctionHandle invoked.  Function Name: "s + functionName);
#endif

    if (!dlHandle_) {
        Log::Error("Library not initialized in call to "
                   "DynamicLoader::getFunctionHandle!");
        return false;
    }

    functionHandle = alpine_dlsym(dlHandle_, functionName.c_str());

    if (!functionHandle) {
        Log::Error("Error locating symbol: "s + functionName + " in library: " + libPath_ +
                   " in call to DynamicLoader::getFunctionHandle!");

        return false;
    }


    return true;
}


bool
DynamicLoader::close()
{
#ifdef _VERBOSE
    Log::Debug("DynamicLoader::close invoked.");
#endif

    if (dlHandle_) {
        alpine_dlclose(dlHandle_);
        dlHandle_ = {};
    }

    return true;
}


void
DynamicLoader::setFlags()
{
    dlFlags_ = 0;

    if (symbolScope_ == t_SymbolScope::Global)
        dlFlags_ = dlFlags_ | RTLD_GLOBAL;

    if (bindingMethod_ == t_BindingMethod::Now)
        dlFlags_ = dlFlags_ | RTLD_NOW;
    else
        dlFlags_ = dlFlags_ | RTLD_LAZY;
}

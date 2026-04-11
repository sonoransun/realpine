/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <Platform.h>


class DynamicLoader
{
  public:
    DynamicLoader();
    ~DynamicLoader();


    enum class t_SymbolScope { Global, Private };

    enum class t_BindingMethod { Now, Lazy };


    bool setSymbolScope(t_SymbolScope scope);

    t_SymbolScope getSymbolScope();

    bool setBindingMethod(t_BindingMethod method);

    t_BindingMethod getBindingMethod();


    bool load(const string & libPath);

    bool getFunctionHandle(const string & functionName, void *& functionHandle);

    bool close();


  private:
    string libPath_;
    t_SymbolScope symbolScope_;
    t_BindingMethod bindingMethod_;
    int dlFlags_;
    alpine_dl_handle dlHandle_;

    void setFlags();
};

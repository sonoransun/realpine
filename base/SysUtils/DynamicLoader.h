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


#pragma once
#include <Common.h>
#include <Platform.h>


class DynamicLoader
{
  public:

    DynamicLoader ();
    ~DynamicLoader ();


    enum class t_SymbolScope { Global, Private };

    enum class t_BindingMethod { Now, Lazy };


    bool  setSymbolScope (t_SymbolScope  scope);

    t_SymbolScope  getSymbolScope ();

    bool  setBindingMethod (t_BindingMethod  method);

    t_BindingMethod  getBindingMethod ();


    bool  load (const string &  libPath);

    bool  getFunctionHandle (const string &  functionName,
                             void *&         functionHandle);

    bool  close ();


  private:

    string              libPath_;
    t_SymbolScope       symbolScope_;
    t_BindingMethod     bindingMethod_;
    int                 dlFlags_;
    alpine_dl_handle    dlHandle_;

    void  setFlags ();
};


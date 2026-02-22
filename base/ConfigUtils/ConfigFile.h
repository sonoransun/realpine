/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <OptHash.h>


class ConfigFile
{
  public:

    ConfigFile ();
    ~ConfigFile ();


    bool  initialize (const string & fileName);

    bool  exists (const string & name);

    bool  get (const string & name,
               string &       value);

    bool  set (const string & name,
               const string & value);

    bool  save ();



    using t_NameIndex = std::unordered_map <string,
                      string,
                      OptHash<string>,
                      equal_to<string> >;

    using t_NameIndexPair = std::pair <string, string>;



  private:

    string         fileName_;
    t_NameIndex *  nameIndex_;
    bool           isDirty_;
    
};


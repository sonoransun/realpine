/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <OptHash.h>


class EnvironMap
{
  public:

    EnvironMap ();
    ~EnvironMap ();

 
    bool  load ();

    bool  exists (const string & name);

    bool  get (const string & name,
               string &       value);



    using t_EnvIndex = std::unordered_map <string,
                      string,
                      OptHash<string>,
                      equal_to<string> >;

    using t_EnvIndexPair = std::pair <string, string>;


  private:

    t_EnvIndex *  envIndex_;

};


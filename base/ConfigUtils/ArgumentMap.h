/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <OptHash.h>


class ArgumentMap
{
  public:

    ArgumentMap ();
    ~ArgumentMap ();

  
    bool  load (int     argc,
                char ** argv);

    bool  exists (const string & name);

    bool  get (const string & name,
               string &       value);



    using t_ArgIndex = std::unordered_map <string,
                      string,
                      OptHash<string>,
                      equal_to<string> >;

    using t_ArgIndexPair = std::pair <string, string>;


  private:

    t_ArgIndex *  argIndex_;

};


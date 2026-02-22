/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>


class AlpineQueryOptionData;


class AlpineQueryOptions
{
  public:

    AlpineQueryOptions ();

    AlpineQueryOptions (const AlpineQueryOptions & copy);

    ~AlpineQueryOptions ();

    const AlpineQueryOptions & operator = (const AlpineQueryOptions & copy);



    bool  setGroup (const string &  groupName);

    bool  getGroup (string &  groupName);

    bool  setAutoHalt (ulong  numHits);

    bool  getAutoHalt (ulong &  numHits);

    bool  setMaxDescPerPeer (ulong  maxDesc);

    bool  getMaxDescPerPeer (ulong &  maxDesc);

    bool  setQuery (const string &  query);

    bool  getQuery (string &  query);

    bool  setOptionId (ulong  optionId);

    bool  getOptionId (ulong &  optionId);

    bool  setOptionData (AlpineQueryOptionData *  optionData);

    bool  getOptionData (AlpineQueryOptionData *&  optionData);



  private:

    string                    group_;
    ulong                     autoHaltLimit_;
    ulong                     maxDescPerPeer_;
    string                    query_;
    ulong                     optionId_;
    AlpineQueryOptionData *   optionData_;
   
};



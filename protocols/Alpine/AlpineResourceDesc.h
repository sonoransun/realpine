/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <string>
#include <vector>


class AlpineQueryOptionData;


class AlpineResourceDesc
{
  public:
    AlpineResourceDesc();

    AlpineResourceDesc(const AlpineResourceDesc & copy);

    ~AlpineResourceDesc();

    AlpineResourceDesc & operator=(const AlpineResourceDesc & copy);


    // Public Types
    //
    using t_LocatorList = vector<string>;


    // Public Methods
    //
    void setMatchId(ulong matchId);

    ulong getMatchId();

    void setSize(ulong size);

    ulong getSize();

    void setLocatorList(const t_LocatorList & locatorList);

    void getLocatorList(t_LocatorList & locatorList);

    void setDescription(const string & description);

    void getDescription(string & description);

    bool setOptionId(ulong optionId);

    ulong getOptionId();

    bool setOptionData(AlpineQueryOptionData * optionData);

    bool getOptionData(AlpineQueryOptionData *& optionData);


  private:
    ulong matchId_;
    ulong size_;
    t_LocatorList locatorList_;
    string description_;
    ulong optionId_;
    AlpineQueryOptionData * optionData_;
};

/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>


class AlpineQueryOptionData;


class AlpineQueryRequest
{
  public:
    AlpineQueryRequest();

    AlpineQueryRequest(const AlpineQueryRequest & copy);

    ~AlpineQueryRequest();

    AlpineQueryRequest & operator=(const AlpineQueryRequest & copy);


    bool setQueryId(ulong queryId);

    ulong getQueryId();

    bool setQueryString(const string & queryString);

    string getQueryString();

    bool setOptionId(ulong optionId);

    ulong getOptionId();

    bool setOptionData(AlpineQueryOptionData * optionData);

    bool getOptionData(AlpineQueryOptionData *& optionData);


  private:
    ulong queryId_;
    string queryString_;
    ulong optionId_;
    AlpineQueryOptionData * optionData_;
};

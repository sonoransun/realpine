/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>


class AlpinePeerProfile
{
  public:
    AlpinePeerProfile();

    AlpinePeerProfile(const AlpinePeerProfile & copy);

    AlpinePeerProfile(ulong peerId);

    ~AlpinePeerProfile();

    AlpinePeerProfile & operator=(const AlpinePeerProfile & copy);


    void getPeerId(ulong & peerId);

    void getRelativeQuality(short & quality);

    void getTotalQueries(ulong & total);

    void getTotalResponses(ulong & total);


  private:
    ulong peerId_;
    short quality_;
    ulong queries_;
    ulong responses_;


    void setPeerId(ulong peerId);

    void setRelativeQuality(short quality);

    void modifyRelativeQuality(short delta);

    void incrTotalQueries();

    void incrTotalResponses();


    friend class AlpinePeerProfileIndex;
};

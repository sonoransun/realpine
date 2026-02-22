/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <string>


class AlpinePeerOptionData;


class AlpinePeerLocation
{
  public:

    AlpinePeerLocation ();

    AlpinePeerLocation (const AlpinePeerLocation & copy);

    ~AlpinePeerLocation ();

    AlpinePeerLocation & operator = (const AlpinePeerLocation & copy);



    // Public Methods
    //
    void  setIpAddress (ulong  ipAddress);

    ulong  getIpAddress ();

    void  setPort (ushort  port);

    ushort  getPort ();

    bool  setOptionId (ulong  optionId);

    ulong  getOptionId ();

    bool  setOptionData (AlpinePeerOptionData *  optionData);

    bool  getOptionData (AlpinePeerOptionData *&  optionData);



  private:

    ulong                   ipAddress_;
    ushort                  port_;
    ulong                   optionId_;
    AlpinePeerOptionData *  optionData_;

};


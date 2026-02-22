/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <vector>
#include <memory>
#include <OptHash.h>


class ConfigData
{
  public:

    // Some types used to store configuration data
    //
    using t_ValueList = vector<string>;

    enum class t_ElementType { String, Number, StringList, NumberList };
        
    struct t_ConfigElement {
        string         elementName;     // internal name for configuration element
        string         fileOptionName;  // config file option name      (1st precedence)
        string         argOptionName;   // command argument option name (2nd precedence)
        string         envOptionName;   // env variable option name     (3rd precedence)
        t_ElementType  optionType;
        bool           required;
    };

    using t_ConfigElementList = vector<t_ConfigElement *>;



    // Methods
    //
    ConfigData ();

    ConfigData (t_ConfigElementList *  elementList);

    ConfigData (const ConfigData &  copy);

    ~ConfigData ();

    ConfigData & operator = (const ConfigData &  copy);



    bool  valueIsSet (const string & name);
                      
    bool  getValue (const string & name,
                    string &       value);

    bool  getValue (const string & name,
                    t_ValueList &  valueList);

    bool  setValue (const string & name,
                    const string & value);

    bool  setValue (const string & name,
                    t_ValueList &  valueList);



    // Internal types
    //
    using t_ElementIndex = std::unordered_map <string,
                      t_ConfigElement *,
                      OptHash<string>,
                      equal_to<string> >;

    using t_ElementIndexPair = std::pair<string, t_ConfigElement *>;


    using t_ValueIndex = std::unordered_map <string,
                      string,
                      OptHash<string>,
                      equal_to<string> >;

    using t_ValueIndexPair = std::pair<string, string>;


    using t_ValueListIndex = std::unordered_map <string,
                      t_ValueList *,
                      OptHash<string>,
                      equal_to<string> >;

    using t_ValueListIndexPair = std::pair<string, t_ValueList *>;



  private:

    std::unique_ptr<t_ConfigElementList>   elementList_;
    std::unique_ptr<t_ElementIndex>        elementIndex_;
    std::unique_ptr<t_ValueIndex>          valueIndex_;
    std::unique_ptr<t_ValueListIndex>      valueListIndex_;

};


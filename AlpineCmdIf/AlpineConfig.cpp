/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineConfig.h>


const string                            AlpineConfig::configFile_s ("alpine.cfg");
ConfigData::t_ConfigElementList *       AlpineConfig::configElements_s = nullptr;


// Ctor defaulted in header


// Dtor defaulted in header



void  
AlpineConfig::createConfigElements ()
{
    if (configElements_s)
        return;

    configElements_s = new ConfigData::t_ConfigElementList;

    ConfigData::t_ConfigElement *  currElement;


    // add configuration elements for test application
    //

    // JSON-RPC server address
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Server Address";
    currElement->argOptionName = "serverAddress";
    currElement->envOptionName = "SERVER_ADDRESS";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = true;
    configElements_s->push_back (currElement);

    // JSON-RPC server port
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Server Port";
    currElement->argOptionName = "serverPort";
    currElement->envOptionName = "SERVER_PORT";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = true;
    configElements_s->push_back (currElement);

    // interface command to execute
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Command";
    currElement->argOptionName = "command";
    currElement->envOptionName = "COMMAND";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = true;
    configElements_s->push_back (currElement);

    // Verbosity on/off
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Verbose";
    currElement->argOptionName = "verbose";
    currElement->envOptionName = "VERBOSE";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);



    //
    // command arguments
    //

    // IP Address
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "IP Address";
    currElement->argOptionName = "ipAddress";
    currElement->envOptionName = "IP_ADDRESS";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // Port
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Port";
    currElement->argOptionName = "port";
    currElement->envOptionName = "PORT";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // Peer ID
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Peer ID";
    currElement->argOptionName = "peerId";
    currElement->envOptionName = "PEER_ID";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // Subnet IP Address
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Subnet IP Address";
    currElement->argOptionName = "subnetIpAddress";
    currElement->envOptionName = "SUBNET_IP_ADDR";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // Subnet mask
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Subnet Mask";
    currElement->argOptionName = "subnetMask";
    currElement->envOptionName = "SUBNET_MASK";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // Required
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Required";
    currElement->argOptionName = "required";
    currElement->envOptionName = "REQUIRED";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // Bits per Second
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Kbits/sec";
    currElement->argOptionName = "Kbits/sec";
    currElement->envOptionName = "KBITS_PER_SEC";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // Limit
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Limit";
    currElement->argOptionName = "limit";
    currElement->envOptionName = "LIMIT";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // Group Name
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Group Name";
    currElement->argOptionName = "groupName";
    currElement->envOptionName = "GROUP_NAME";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // Group ID
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Group ID";
    currElement->argOptionName = "groupId";
    currElement->envOptionName = "GROUP_ID";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // Alias
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Alias";
    currElement->argOptionName = "alias";
    currElement->envOptionName = "ALIAS";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // Description
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Description";
    currElement->argOptionName = "description";
    currElement->envOptionName = "DESCRIPTION";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // Rating
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Rating";
    currElement->argOptionName = "rating";
    currElement->envOptionName = "RATING";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // Public
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Public";
    currElement->argOptionName = "public";
    currElement->envOptionName = "PUBLIC";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // Private
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Private";
    currElement->argOptionName = "private";
    currElement->envOptionName = "PRIVATE";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // Auto Halt Limit
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Auto Halt Limit";
    currElement->argOptionName = "autoHaltLimit";
    currElement->envOptionName = "AUTO_HALT_LIMIT";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // Options
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Options";
    currElement->argOptionName = "options";
    currElement->envOptionName = "OPTIONS";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // Auto Download
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Auto Download";
    currElement->argOptionName = "autoDownload";
    currElement->envOptionName = "AUTO_DOWNLOAD";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // Peer Description Limit
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Peer Description Limit";
    currElement->argOptionName = "peerDescriptionLimit";
    currElement->envOptionName = "PEER_DESC_LIMIT";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // Query Option ID
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Query Option ID";
    currElement->argOptionName = "queryOptionId";
    currElement->envOptionName = "QUERY_OPT_ID";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // Query Option Data
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Query Option Data";
    currElement->argOptionName = "queryOptionData";
    currElement->envOptionName = "QUERY_OPT_DATA";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // Query String
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Query String";
    currElement->argOptionName = "queryString";
    currElement->envOptionName = "QUERY_STRING";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // Query ID
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Query ID";
    currElement->argOptionName = "queryId";
    currElement->envOptionName = "QUERY_ID";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // Module ID
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Module ID";
    currElement->argOptionName = "moduleId";
    currElement->envOptionName = "MODULE_ID";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // Module Library Path
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Module Lib Path";
    currElement->argOptionName = "moduleLibPath";
    currElement->envOptionName = "MODULE_LIB_PATH";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // Module Boostrap Symbol
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Module Symbol";
    currElement->argOptionName = "moduleSymbol";
    currElement->envOptionName = "MODULE_SYMBOL";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);
}



void  
AlpineConfig::getConfigElements (ConfigData::t_ConfigElementList *& configElements)
{
    configElements = configElements_s;
}



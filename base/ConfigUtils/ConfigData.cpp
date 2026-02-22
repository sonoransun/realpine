/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <ConfigData.h>
#include <Log.h>



ConfigData::ConfigData ()
{
    elementList_    = std::make_unique<t_ConfigElementList>();
    elementIndex_   = std::make_unique<t_ElementIndex>();
    valueIndex_     = std::make_unique<t_ValueIndex>();
    valueListIndex_ = std::make_unique<t_ValueListIndex>();
}



ConfigData::ConfigData (t_ConfigElementList *  elementList)
{
    elementList_    = std::make_unique<t_ConfigElementList>();

    t_ConfigElement * currElement;
    t_ConfigElement * copyElement;

    for (const auto& item : *elementList) {

        currElement = item;
        copyElement = new t_ConfigElement (*currElement);
        elementList_->push_back (copyElement);
    }


    elementIndex_   = std::make_unique<t_ElementIndex>();
    valueIndex_     = std::make_unique<t_ValueIndex>();
    valueListIndex_ = std::make_unique<t_ValueListIndex>();


    for (const auto& item : *elementList_) {

        currElement = item;
        elementIndex_->emplace (currElement->elementName, currElement);
    }
}



ConfigData::ConfigData (const ConfigData & copy)
{
    elementList_    = std::make_unique<t_ConfigElementList>();

    t_ConfigElement * currElement;
    t_ConfigElement * copyElement;

    for (const auto& item : *(copy.elementList_)) {

        currElement = item;
        copyElement = new t_ConfigElement (*currElement);
        elementList_->push_back (copyElement);
    }


    elementIndex_   = std::make_unique<t_ElementIndex>();
    valueIndex_     = std::make_unique<t_ValueIndex>(*(copy.valueIndex_));
    valueListIndex_ = std::make_unique<t_ValueListIndex>();

    if (copy.valueListIndex_->size () > 0) {
        t_ValueList *  newList;

        for (const auto& [key, value] : *(copy.valueListIndex_)) {

             newList = new t_ValueList (*value);
             valueListIndex_->emplace (key, newList);
        }
        return;
    }


    for (const auto& item : *elementList_) {

        currElement = item;
        elementIndex_->emplace (currElement->elementName, currElement);
    }
}



ConfigData::~ConfigData ()
{
    if (elementList_) {
        for (const auto& item : *elementList_) {
            delete item;
        }
        return;
    }

    if (valueListIndex_) {
        for (const auto& [key, value] : *valueListIndex_) {
            delete value;
        }
    }
}



ConfigData &
ConfigData::operator = (const ConfigData &  copy)
{
    if (&copy == this) {
        return *this;
    }

    if (elementList_) {

        for (const auto& item : *elementList_) {
            delete item;
        }
        return *this;
    }

    elementList_ = std::make_unique<t_ConfigElementList>();

    t_ConfigElement * currElement;
    t_ConfigElement * copyElement;

    for (const auto& item : *(copy.elementList_)) {

        currElement = item;
        copyElement = new t_ConfigElement (*currElement);
        elementList_->push_back (copyElement);
    }


    if (valueListIndex_) {
        for (const auto& [key, value] : *valueListIndex_) {
            delete value;
        }
        return *this;
    }


    elementIndex_   = std::make_unique<t_ElementIndex>();
    valueIndex_     = std::make_unique<t_ValueIndex>(*(copy.valueIndex_));
    valueListIndex_ = std::make_unique<t_ValueListIndex>();

    if (copy.valueListIndex_->size () > 0) {
        t_ValueList *  newList;

        for (const auto& [key, value] : *(copy.valueListIndex_)) {

             newList = new t_ValueList (*value);
             valueListIndex_->emplace (key, newList);
        }
        return *this;
    }


    for (const auto& item : *elementList_) {

        currElement = item;
        elementIndex_->emplace (currElement->elementName, currElement);
    }


    return *this;
}



bool  
ConfigData::valueIsSet (const string & name)
{
    t_ConfigElement * currElement;

    auto iter = elementIndex_->find (name);

    if (iter == elementIndex_->end ()) {
        return false;
    }
    currElement = (*iter).second;

    if ( (currElement->optionType == t_ElementType::StringList) ||
         (currElement->optionType == t_ElementType::NumberList) ) {

        auto valueIter = valueListIndex_->find (name);

        if (valueIter == valueListIndex_->end ()) {
            return false;
        }
    }
    else {

        auto valueIter = valueIndex_->find (name);

        if (valueIter == valueIndex_->end ()) {
            return false;
        }
    }


    return true;
}


                      
bool  
ConfigData::getValue (const string & name,
                      string &       value)
{
    auto valueIter = valueIndex_->find (name);

    if (valueIter == valueIndex_->end ()) {
        return false;
    }
    value = (*valueIter).second;


    return true;
}



bool  
ConfigData::getValue (const string & name,
                      t_ValueList &  valueList)
{
    auto valueIter = valueListIndex_->find (name);

    if (valueIter == valueListIndex_->end ()) {
        return false;
    }
    t_ValueList *  valueListPtr;
    valueListPtr = (*valueIter).second;

    valueList = *valueListPtr;


    return true;
}



bool  
ConfigData::setValue (const string & name,
                      const string & value)
{
    t_ConfigElement * currElement;

    auto iter = elementIndex_->find (name);

    if (iter == elementIndex_->end ()) {
        return false;
    }
    currElement = (*iter).second;

    if ( (currElement->optionType == t_ElementType::StringList) ||
         (currElement->optionType == t_ElementType::NumberList) ) {

        Log::Error ("Attempt to set configuration value to invalid type!");
        return false;
    }
    valueIndex_->emplace (name, value);
    

    return true;
}



bool  
ConfigData::setValue (const string & name,
                      t_ValueList &  valueList)
{
    t_ConfigElement * currElement;

    auto iter = elementIndex_->find (name);

    if (iter == elementIndex_->end ()) {
        return false;
    }
    currElement = (*iter).second;

    if ( (currElement->optionType == t_ElementType::String) ||
         (currElement->optionType == t_ElementType::Number) ) {

        Log::Error ("Attempt to set configuration value to invalid type!");
        return false;
    }
    t_ValueList *  newValueList;
    newValueList = new t_ValueList(valueList);

    valueListIndex_->emplace (name, newValueList);
    

    return true;
}




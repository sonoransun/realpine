/// Unit tests for Configuration basics

#include <ConfigData.h>
#include <catch2/catch_test_macros.hpp>


TEST_CASE("ConfigData getValue and setValue", "[ConfigData]")
{
    // Set up a minimal config element
    ConfigData::t_ConfigElement elem;
    elem.elementName = "TestParam";
    elem.fileOptionName = "test_param";
    elem.argOptionName = "--test-param";
    elem.envOptionName = "TEST_PARAM";
    elem.optionType = ConfigData::t_ElementType::String;
    elem.required = false;

    ConfigData::t_ConfigElementList elements;
    elements.push_back(&elem);

    ConfigData data(&elements);

    SECTION("getValue returns false for unset parameter")
    {
        string val;
        REQUIRE_FALSE(data.getValue("TestParam", val));
    }

    SECTION("setValue and getValue roundtrip")
    {
        bool setOk = data.setValue("TestParam", "hello_test"s);
        REQUIRE(setOk);

        string getVal;
        bool getOk = data.getValue("TestParam", getVal);
        REQUIRE(getOk);
        REQUIRE(getVal == "hello_test");
    }

    SECTION("setValue keeps first value set")
    {
        data.setValue("TestParam", "first"s);
        data.setValue("TestParam", "second"s);

        string val;
        REQUIRE(data.getValue("TestParam", val));
        REQUIRE(val == "first");
    }

    SECTION("getValue for nonexistent element returns false")
    {
        string val;
        REQUIRE_FALSE(data.getValue("NoSuchParam", val));
    }

    SECTION("valueIsSet returns false for unset, true after set")
    {
        REQUIRE_FALSE(data.valueIsSet("TestParam"));

        data.setValue("TestParam", "value"s);
        REQUIRE(data.valueIsSet("TestParam"));
    }
}


TEST_CASE("ConfigData value list operations", "[ConfigData]")
{
    ConfigData::t_ConfigElement elem;
    elem.elementName = "ListParam";
    elem.fileOptionName = "list_param";
    elem.argOptionName = "--list-param";
    elem.envOptionName = "LIST_PARAM";
    elem.optionType = ConfigData::t_ElementType::StringList;
    elem.required = false;

    ConfigData::t_ConfigElementList elements;
    elements.push_back(&elem);

    ConfigData data(&elements);

    SECTION("setValue and getValue with value list")
    {
        ConfigData::t_ValueList setList = {"alpha", "beta", "gamma"};
        bool setOk = data.setValue("ListParam", setList);
        REQUIRE(setOk);

        ConfigData::t_ValueList getList;
        bool getOk = data.getValue("ListParam", getList);
        REQUIRE(getOk);
        REQUIRE(getList.size() == 3);
    }
}

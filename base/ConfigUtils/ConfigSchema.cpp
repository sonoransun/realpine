/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <ConfigSchema.h>
#include <Log.h>
#include <stdexcept>


std::unordered_map<string, ConfigSchema::t_FieldSpec>  ConfigSchema::schema_s;



void
ConfigSchema::addField (const string &       name,
                        const t_FieldSpec &   spec)
{
    schema_s[name] = spec;
}



bool
ConfigSchema::validate (const string &  name,
                        const string &  value,
                        string &        errorMsg)
{
    auto it = schema_s.find(name);
    if (it == schema_s.end())
        return true;  // No schema rule — accept

    const auto & spec = it->second;

    if (spec.validator && !spec.validator(value)) {
        errorMsg = "Custom validation failed for: "s + name;
        return false;
    }

    switch (spec.type) {
        case t_ValueType::Number:
            return validateNumber(value, spec, errorMsg);
        case t_ValueType::Boolean:
            return validateBoolean(value, errorMsg);
        case t_ValueType::String:
            break;
    }

    return true;
}



bool
ConfigSchema::validateAll (const std::unordered_map<string, string> &  values,
                           vector<string> &                            errors)
{
    bool allValid = true;

    // Check required fields
    for (const auto & [name, spec] : schema_s) {
        if (spec.required && !values.contains(name)) {
            errors.push_back("Required field missing: "s + name);
            allValid = false;
        }
    }

    // Validate provided values
    for (const auto & [name, value] : values) {
        string errorMsg;
        if (!validate(name, value, errorMsg)) {
            errors.push_back(errorMsg);
            allValid = false;
        }
    }

    return allValid;
}



void
ConfigSchema::clear ()
{
    schema_s.clear();
}



bool
ConfigSchema::validateNumber (const string &       value,
                              const t_FieldSpec &  spec,
                              string &             errorMsg)
{
    long long numValue;
    try {
        numValue = std::stoll(value);
    } catch (...) {
        errorMsg = "Expected numeric value, got: "s + value;
        return false;
    }

    if (spec.minValue) {
        long long minVal = std::stoll(*spec.minValue);
        if (numValue < minVal) {
            errorMsg = "Value "s + value + " below minimum "s + *spec.minValue;
            return false;
        }
    }

    if (spec.maxValue) {
        long long maxVal = std::stoll(*spec.maxValue);
        if (numValue > maxVal) {
            errorMsg = "Value "s + value + " above maximum "s + *spec.maxValue;
            return false;
        }
    }

    return true;
}



bool
ConfigSchema::validateBoolean (const string &  value,
                               string &        errorMsg)
{
    if (value == "true" || value == "false" ||
        value == "1" || value == "0" ||
        value == "yes" || value == "no") {
        return true;
    }

    errorMsg = "Expected boolean value, got: "s + value;
    return false;
}

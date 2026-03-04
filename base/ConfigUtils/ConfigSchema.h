/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <functional>
#include <optional>
#include <unordered_map>


class ConfigSchema
{
  public:

    enum class t_ValueType { String, Number, Boolean };

    struct t_FieldSpec {
        t_ValueType                    type      = t_ValueType::String;
        bool                           required  = false;
        std::optional<string>          minValue;
        std::optional<string>          maxValue;
        std::optional<string>          defaultValue;
        std::function<bool(const string&)>  validator;
    };


    static void  addField (const string &       name,
                           const t_FieldSpec &   spec);

    static bool  validate (const string &  name,
                           const string &  value,
                           string &        errorMsg);

    static bool  validateAll (const std::unordered_map<string, string> &  values,
                              vector<string> &                            errors);

    static void  clear ();


  private:

    static bool  validateNumber (const string &       value,
                                 const t_FieldSpec &  spec,
                                 string &             errorMsg);

    static bool  validateBoolean (const string &  value,
                                  string &        errorMsg);

    static std::unordered_map<string, t_FieldSpec>  schema_s;

};

/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>


class OutputFormatter
{
  public:
    static string formatTable(const vector<string> & headers, const vector<vector<string>> & rows);

    static string formatCsv(const vector<string> & headers, const vector<vector<string>> & rows);

    static string formatYaml(const vector<string> & headers, const vector<vector<string>> & rows);
};

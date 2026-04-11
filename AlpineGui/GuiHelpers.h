/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once

#include <AppState.h>
#include <string>
#include <vector>


void addLog(AppState & state, LogLevel level, const std::string & msg);

std::string formatJson(const std::string & json);

void parseJsonArray(const std::string & json, std::vector<std::string> & items);

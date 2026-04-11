/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>


class ShellCompletion
{
  public:
    /// Generate a bash completion script for the given commands.
    /// Each pair is (command_name, description).
    static string generateBash(const vector<pair<string, string>> & commands);

    /// Generate a zsh completion script for the given commands.
    /// Each pair is (command_name, description).
    static string generateZsh(const vector<pair<string, string>> & commands);
};

/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <OptHash.h>
#include <chrono>
#include <memory>
#include <string_view>
#include <unordered_map>


class VfsNode
{
  public:
    enum class t_NodeType { Directory, File };

    string name;
    t_NodeType nodeType;
    ulong size = 0;
    ulong peerId = 0;
    ulong resourceId = 0;
    ulong queryId = 0;
    string queryTerm;
    string locatorUrl;

    std::chrono::system_clock::time_point createdAt;
    std::chrono::system_clock::time_point accessedAt;

    VfsNode * parent = nullptr;

    std::unordered_map<string, std::unique_ptr<VfsNode>, OptHash<string>> children;


    VfsNode(std::string_view nodeName, t_NodeType type, VfsNode * parentNode = nullptr);

    VfsNode * addChild(std::string_view childName, t_NodeType type);

    VfsNode * findChild(std::string_view childName);

    bool removeChild(std::string_view childName);

    string fullPath() const;

    bool isDirectory() const;
};

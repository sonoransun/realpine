/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <VfsNode.h>


VfsNode::VfsNode (std::string_view  nodeName,
                  t_NodeType        type,
                  VfsNode *         parentNode)
    : name       (nodeName),
      nodeType   (type),
      createdAt  (std::chrono::system_clock::now()),
      accessedAt (std::chrono::system_clock::now()),
      parent     (parentNode)
{
}


VfsNode *
VfsNode::addChild (std::string_view  childName,
                   t_NodeType        type)
{
    auto key = string(childName);

    if (children.contains(key))
        return children[key].get();

    auto node    = std::make_unique<VfsNode>(childName, type, this);
    auto * raw   = node.get();
    children[key] = std::move(node);

    return raw;
}


VfsNode *
VfsNode::findChild (std::string_view  childName)
{
    auto key = string(childName);
    auto it  = children.find(key);

    if (it == children.end())
        return nullptr;

    it->second->accessedAt = std::chrono::system_clock::now();
    return it->second.get();
}


bool
VfsNode::removeChild (std::string_view  childName)
{
    auto key = string(childName);
    return children.erase(key) > 0;
}


string
VfsNode::fullPath () const
{
    if (!parent)
        return "/"s + name;

    auto path = parent->fullPath();

    if (path.back() != '/')
        path += '/';

    path += name;
    return path;
}


bool
VfsNode::isDirectory () const
{
    return nodeType == t_NodeType::Directory;
}

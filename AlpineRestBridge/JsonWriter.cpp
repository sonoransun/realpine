/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <JsonWriter.h>


JsonWriter::JsonWriter ()
    : root_(nullptr)
{
}


string
JsonWriter::result ()
{
    return root_.dump();
}


void
JsonWriter::beginObject ()
{
    if (stack_.empty()) {
        root_ = nlohmann::json::object();
        stack_.push(&root_);
    } else {
        auto * parent = stack_.top();
        if (parent->is_array()) {
            parent->push_back(nlohmann::json::object());
            stack_.push(&parent->back());
        } else if (parent->is_object() && !currentKey_.empty()) {
            (*parent)[currentKey_] = nlohmann::json::object();
            stack_.push(&(*parent)[currentKey_]);
            currentKey_.clear();
        }
    }
}


void
JsonWriter::endObject ()
{
    if (!stack_.empty())
        stack_.pop();
}


void
JsonWriter::beginArray ()
{
    if (stack_.empty()) {
        root_ = nlohmann::json::array();
        stack_.push(&root_);
    } else {
        auto * parent = stack_.top();
        if (parent->is_array()) {
            parent->push_back(nlohmann::json::array());
            stack_.push(&parent->back());
        } else if (parent->is_object() && !currentKey_.empty()) {
            (*parent)[currentKey_] = nlohmann::json::array();
            stack_.push(&(*parent)[currentKey_]);
            currentKey_.clear();
        }
    }
}


void
JsonWriter::endArray ()
{
    if (!stack_.empty())
        stack_.pop();
}


void
JsonWriter::key (const string & k)
{
    currentKey_ = k;
}


void
JsonWriter::value (const string & v)
{
    if (stack_.empty())
        return;
    auto * current = stack_.top();
    if (current->is_array()) {
        current->push_back(v);
    } else if (current->is_object() && !currentKey_.empty()) {
        (*current)[currentKey_] = v;
        currentKey_.clear();
    }
}


void
JsonWriter::value (ulong v)
{
    if (stack_.empty())
        return;
    auto * current = stack_.top();
    if (current->is_array()) {
        current->push_back(v);
    } else if (current->is_object() && !currentKey_.empty()) {
        (*current)[currentKey_] = v;
        currentKey_.clear();
    }
}


void
JsonWriter::value (bool v)
{
    if (stack_.empty())
        return;
    auto * current = stack_.top();
    if (current->is_array()) {
        current->push_back(v);
    } else if (current->is_object() && !currentKey_.empty()) {
        (*current)[currentKey_] = v;
        currentKey_.clear();
    }
}


void
JsonWriter::separator ()
{
    // no-op: nlohmann handles separators automatically
}

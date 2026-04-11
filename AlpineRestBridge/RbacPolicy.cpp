/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <Log.h>
#include <RbacPolicy.h>
#include <format>
#include <fstream>
#include <mutex>
#include <nlohmann/json.hpp>


std::unordered_map<string, std::unordered_set<string>> RbacPolicy::roles_s;
std::unordered_map<string, string> RbacPolicy::keyRoles_s;
std::shared_mutex RbacPolicy::policyMutex_s;
string RbacPolicy::policyFilePath_s;
bool RbacPolicy::enabled_s = false;


bool
RbacPolicy::initialize(const string & policyFilePath)
{
    policyFilePath_s = policyFilePath;
    if (policyFilePath.empty()) {
        Log::Info("RbacPolicy: no policy file configured, RBAC disabled");
        return true;
    }

    if (!loadPolicy(policyFilePath)) {
        return false;
    }

    enabled_s = true;
    Log::Info("RbacPolicy: loaded {} roles, {} key mappings", roles_s.size(), keyRoles_s.size());
    return true;
}


bool
RbacPolicy::hasPermission(const string & apiKey, const string & permission)
{
    if (!enabled_s) {
        return true;  // RBAC disabled → all access granted
    }

    std::shared_lock lock(policyMutex_s);

    auto keyIt = keyRoles_s.find(apiKey);
    if (keyIt == keyRoles_s.end()) {
        return false;  // unknown key → no access
    }

    auto roleIt = roles_s.find(keyIt->second);
    if (roleIt == roles_s.end()) {
        return false;  // key mapped to nonexistent role
    }

    const auto & perms = roleIt->second;
    return perms.contains("*"s) || perms.contains(permission);
}


string
RbacPolicy::getRoleForKey(const string & apiKey)
{
    if (!enabled_s) {
        return "admin"s;
    }

    std::shared_lock lock(policyMutex_s);

    auto it = keyRoles_s.find(apiKey);
    if (it != keyRoles_s.end()) {
        return it->second;
    }
    return ""s;
}


bool
RbacPolicy::isEnabled()
{
    return enabled_s;
}


bool
RbacPolicy::reload()
{
    if (policyFilePath_s.empty()) {
        return false;
    }
    return loadPolicy(policyFilePath_s);
}


bool
RbacPolicy::loadPolicy(const string & filePath)
{
    try {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            Log::Error("RbacPolicy: cannot open policy file: {}", filePath);
            return false;
        }

        auto doc = nlohmann::json::parse(file);

        std::unique_lock lock(policyMutex_s);

        // Parse roles
        roles_s.clear();
        if (doc.contains("roles") && doc["roles"].is_object()) {
            for (const auto & [roleName, permsArray] : doc["roles"].items()) {
                std::unordered_set<string> perms;
                if (permsArray.is_array()) {
                    for (const auto & p : permsArray) {
                        if (p.is_string()) {
                            perms.insert(p.get<string>());
                        }
                    }
                }
                roles_s[roleName] = std::move(perms);
            }
        }

        // Parse key-to-role mappings
        keyRoles_s.clear();
        if (doc.contains("keys") && doc["keys"].is_object()) {
            for (const auto & [key, role] : doc["keys"].items()) {
                if (role.is_string()) {
                    keyRoles_s[key] = role.get<string>();
                }
            }
        }

        return true;
    } catch (const nlohmann::json::exception & e) {
        Log::Error("RbacPolicy: JSON parse error in {}: {}", filePath, e.what());
        return false;
    } catch (const std::exception & e) {
        Log::Error("RbacPolicy: error loading {}: {}", filePath, e.what());
        return false;
    }
}

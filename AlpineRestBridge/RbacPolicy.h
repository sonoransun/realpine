/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <unordered_map>
#include <unordered_set>
#include <shared_mutex>


class RbacPolicy
{
  public:

    // Load RBAC policy from a JSON file.
    // Format: { "roles": { "admin": ["*"], "query": ["query:start", "query:cancel", ...] },
    //           "keys": { "abc123...": "admin", "def456...": "query" } }
    static bool  initialize (const string & policyFilePath);

    // Check if a given API key has the specified permission.
    // Permission format: "resource:action" (e.g., "query:start", "admin:ban", "peer:view")
    // Returns true if: key is mapped to a role that includes the permission or "*".
    [[nodiscard]] static bool  hasPermission (const string & apiKey,
                                               const string & permission);

    // Get the role name for an API key. Returns empty string if unknown.
    [[nodiscard]] static string  getRoleForKey (const string & apiKey);

    // Check if RBAC is enabled (policy file loaded successfully).
    [[nodiscard]] static bool  isEnabled ();

    // Reload the policy file.
    static bool  reload ();


  private:

    // role name → set of permissions (or {"*"} for full access)
    static std::unordered_map<string, std::unordered_set<string>>  roles_s;

    // API key → role name
    static std::unordered_map<string, string>                      keyRoles_s;

    static std::shared_mutex                                       policyMutex_s;
    static string                                                  policyFilePath_s;
    static bool                                                    enabled_s;

    static bool  loadPolicy (const string & filePath);

};

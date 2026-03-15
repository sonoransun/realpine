#!/usr/bin/env bash
# Alpine CLI (alpc) — Bash completion
# Install: source <(AlpineCmdIf --completions bash)
#   or:    cp alpc.bash /etc/bash_completion.d/alpc
#   or:    cp alpc.bash ~/.local/share/bash-completion/completions/alpc

# Alias setup — use 'alpc' as shorthand for AlpineCmdIf
alias alpc='AlpineCmdIf'

# Convenience aliases for frequent operations
alias alpc-peers='AlpineCmdIf --command getExtendedPeerList --json --quiet'
alias alpc-status='AlpineCmdIf --command getStatus --json --quiet'
alias alpc-query='AlpineCmdIf --command beginQuery --json --quiet'
alias alpc-results='AlpineCmdIf --command getQueryResults --json --quiet'
alias alpc-modules='AlpineCmdIf --command listActiveModules --json --quiet'
alias alpc-groups='AlpineCmdIf --command getUserGroupList --json --quiet'
alias alpc-interactive='AlpineCmdIf --command interactive'

_alpc_commands() {
    echo "addDtcpPeer getDtcpPeerId getDtcpPeerStatus activateDtcpPeer deactivateDtcpPeer pingDtcpPeer"
    echo "excludeHost excludeSubnet allowHost allowSubnet listExcludedHosts listExcludedSubnets"
    echo "getUserGroupList createUserGroup destroyUserGroup getPeerUserGroupList addPeerToGroup removePeerFromGroup"
    echo "getExtendedPeerList"
    echo "beginQuery getQueryStatus pauseQuery resumeQuery cancelQuery getQueryResults"
    echo "registerModule unregisterModule loadModule unloadModule listActiveModules listAllModules getModuleInfo"
    echo "getStatus interactive help"
}

_alpc_completions() {
    local cur prev pprev commands flags
    COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"
    [[ $COMP_CWORD -ge 2 ]] && pprev="${COMP_WORDS[COMP_CWORD-2]}"

    commands="$(_alpc_commands)"

    # Global flags
    flags="--serverAddress --serverPort --command --verbose --json --quiet --format --color --completions --help --version"

    # --- Subcommand-aware argument completion ---

    # Find the active command (value following --command)
    local active_cmd=""
    for ((i=1; i < COMP_CWORD; i++)); do
        if [[ "${COMP_WORDS[i]}" == "--command" && $((i+1)) -lt ${#COMP_WORDS[@]} ]]; then
            active_cmd="${COMP_WORDS[i+1]}"
            break
        fi
    done

    # Flag value completions
    case "$prev" in
        --command)
            COMPREPLY=( $(compgen -W "$commands" -- "$cur") )
            return 0
            ;;
        --format)
            COMPREPLY=( $(compgen -W "json table csv yaml" -- "$cur") )
            return 0
            ;;
        --completions)
            COMPREPLY=( $(compgen -W "bash zsh" -- "$cur") )
            return 0
            ;;
        --serverAddress)
            COMPREPLY=( $(compgen -W "localhost 127.0.0.1 ::1" -- "$cur") )
            return 0
            ;;
        --serverPort)
            COMPREPLY=( $(compgen -W "8080 9000 8089" -- "$cur") )
            return 0
            ;;
        --logLevel)
            COMPREPLY=( $(compgen -W "Silent Error Info Debug" -- "$cur") )
            return 0
            ;;
    esac

    # Per-command argument suggestions
    case "$active_cmd" in
        addDtcpPeer)
            if [[ "$cur" == -* ]]; then
                COMPREPLY=( $(compgen -W "--ipAddress --port" -- "$cur") )
            fi
            return 0
            ;;
        getDtcpPeerId)
            if [[ "$cur" == -* ]]; then
                COMPREPLY=( $(compgen -W "--ipAddress --port" -- "$cur") )
            fi
            return 0
            ;;
        getDtcpPeerStatus|activateDtcpPeer|deactivateDtcpPeer|pingDtcpPeer)
            if [[ "$cur" == -* ]]; then
                COMPREPLY=( $(compgen -W "--peerId" -- "$cur") )
            fi
            return 0
            ;;
        excludeHost|allowHost)
            if [[ "$cur" == -* ]]; then
                COMPREPLY=( $(compgen -W "--ipAddress" -- "$cur") )
            fi
            return 0
            ;;
        excludeSubnet|allowSubnet)
            if [[ "$cur" == -* ]]; then
                COMPREPLY=( $(compgen -W "--subnetIpAddress --subnetMask" -- "$cur") )
            fi
            return 0
            ;;
        createUserGroup|destroyUserGroup)
            if [[ "$cur" == -* ]]; then
                COMPREPLY=( $(compgen -W "--groupName" -- "$cur") )
            fi
            return 0
            ;;
        getPeerUserGroupList|addPeerToGroup|removePeerFromGroup)
            if [[ "$cur" == -* ]]; then
                COMPREPLY=( $(compgen -W "--peerId --groupName" -- "$cur") )
            fi
            return 0
            ;;
        beginQuery)
            if [[ "$cur" == -* ]]; then
                COMPREPLY=( $(compgen -W "--queryString --numHits --moduleId" -- "$cur") )
            fi
            return 0
            ;;
        getQueryStatus|pauseQuery|resumeQuery|cancelQuery|getQueryResults)
            if [[ "$cur" == -* ]]; then
                COMPREPLY=( $(compgen -W "--queryId" -- "$cur") )
            fi
            return 0
            ;;
        registerModule|loadModule)
            if [[ "$cur" == -* ]]; then
                COMPREPLY=( $(compgen -W "--moduleName --modulePath" -- "$cur") )
            elif [[ "$prev" == "--modulePath" ]]; then
                COMPREPLY=( $(compgen -f -X '!*.so' -- "$cur") )
            fi
            return 0
            ;;
        unregisterModule|unloadModule|getModuleInfo)
            if [[ "$cur" == -* ]]; then
                COMPREPLY=( $(compgen -W "--moduleId" -- "$cur") )
            fi
            return 0
            ;;
        help)
            COMPREPLY=( $(compgen -W "$commands" -- "$cur") )
            return 0
            ;;
    esac

    # Default: complete flags or commands
    if [[ "$cur" == -* ]]; then
        COMPREPLY=( $(compgen -W "$flags" -- "$cur") )
    else
        COMPREPLY=( $(compgen -W "$commands" -- "$cur") )
    fi
}

complete -o default -F _alpc_completions alpc
complete -o default -F _alpc_completions AlpineCmdIf

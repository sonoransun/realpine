#compdef alpc AlpineCmdIf
# Alpine CLI (alpc) — Zsh completion
# Install: fpath=(~/.zsh/completions $fpath); autoload -Uz compinit; compinit
#   or:    cp alpc.zsh ~/.zsh/completions/_alpc

# Alias setup
alias alpc='AlpineCmdIf'
alias alpc-peers='AlpineCmdIf --command getExtendedPeerList --json --quiet'
alias alpc-status='AlpineCmdIf --command getStatus --json --quiet'
alias alpc-query='AlpineCmdIf --command beginQuery --json --quiet'
alias alpc-results='AlpineCmdIf --command getQueryResults --json --quiet'
alias alpc-modules='AlpineCmdIf --command listActiveModules --json --quiet'
alias alpc-groups='AlpineCmdIf --command getUserGroupList --json --quiet'
alias alpc-interactive='AlpineCmdIf --command interactive'

_alpc() {
    local -a commands peer_commands network_commands group_commands
    local -a query_commands module_commands system_commands
    local -a formats shells log_levels

    # Categorized commands for _describe grouping
    peer_commands=(
        'addDtcpPeer:Add a DTCP peer by IP and port'
        'getDtcpPeerId:Get the peer ID for a given address'
        'getDtcpPeerStatus:Get status details for a peer'
        'activateDtcpPeer:Activate a peer connection'
        'deactivateDtcpPeer:Deactivate a peer connection'
        'pingDtcpPeer:Ping a peer to check connectivity'
        'getExtendedPeerList:List all known peers with details'
    )

    network_commands=(
        'excludeHost:Exclude a host by IP address'
        'excludeSubnet:Exclude a subnet'
        'allowHost:Allow a previously excluded host'
        'allowSubnet:Allow a previously excluded subnet'
        'listExcludedHosts:List all excluded hosts'
        'listExcludedSubnets:List all excluded subnets'
    )

    group_commands=(
        'getUserGroupList:List all user groups'
        'createUserGroup:Create a new user group'
        'destroyUserGroup:Destroy a user group'
        'getPeerUserGroupList:List peers in a group'
        'addPeerToGroup:Add a peer to a group'
        'removePeerFromGroup:Remove a peer from a group'
    )

    query_commands=(
        'beginQuery:Start a new distributed query'
        'getQueryStatus:Get status of a running query'
        'pauseQuery:Pause a running query'
        'resumeQuery:Resume a paused query'
        'cancelQuery:Cancel a running query'
        'getQueryResults:Get results from a query'
    )

    module_commands=(
        'registerModule:Register a module for loading'
        'unregisterModule:Unregister a module'
        'loadModule:Load a registered module'
        'unloadModule:Unload a module'
        'listActiveModules:List all active modules'
        'listAllModules:List all registered modules'
        'getModuleInfo:Get details for a module'
    )

    system_commands=(
        'getStatus:Get server status and health'
        'interactive:Start interactive REPL mode'
        'help:Show help for a command'
    )

    formats=(json table csv yaml)
    shells=(bash zsh)
    log_levels=(Silent Error Info Debug)

    # Find the active --command value for subcommand-specific args
    local active_cmd=""
    local i
    for ((i=1; i < $CURRENT; i++)); do
        if [[ "${words[i]}" == "--command" && $((i+1)) -le ${#words} ]]; then
            active_cmd="${words[i+1]}"
            break
        fi
    done

    # Subcommand-specific argument handling
    case "$active_cmd" in
        addDtcpPeer|getDtcpPeerId)
            _arguments -s \
                '--ipAddress[Peer IP address]:address:' \
                '--port[Peer port number]:port:' \
                '*:' && return
            ;;
        getDtcpPeerStatus|activateDtcpPeer|deactivateDtcpPeer|pingDtcpPeer)
            _arguments -s \
                '--peerId[Peer ID]:peer_id:' \
                '*:' && return
            ;;
        excludeHost|allowHost)
            _arguments -s \
                '--ipAddress[Host IP address]:address:' \
                '*:' && return
            ;;
        excludeSubnet|allowSubnet)
            _arguments -s \
                '--subnetIpAddress[Subnet IP]:address:' \
                '--subnetMask[Subnet mask]:mask:' \
                '*:' && return
            ;;
        createUserGroup|destroyUserGroup)
            _arguments -s \
                '--groupName[Group name]:name:' \
                '*:' && return
            ;;
        addPeerToGroup|removePeerFromGroup|getPeerUserGroupList)
            _arguments -s \
                '--peerId[Peer ID]:peer_id:' \
                '--groupName[Group name]:name:' \
                '*:' && return
            ;;
        beginQuery)
            _arguments -s \
                '--queryString[Search query string]:query:' \
                '--numHits[Max number of results]:count:' \
                '--moduleId[Module ID to target]:module_id:' \
                '*:' && return
            ;;
        getQueryStatus|pauseQuery|resumeQuery|cancelQuery|getQueryResults)
            _arguments -s \
                '--queryId[Query ID]:query_id:' \
                '*:' && return
            ;;
        registerModule|loadModule)
            _arguments -s \
                '--moduleName[Module name]:name:' \
                '--modulePath[Path to module .so/.dll]:path:_files -g "*.so *.dll *.dylib"' \
                '*:' && return
            ;;
        unregisterModule|unloadModule|getModuleInfo)
            _arguments -s \
                '--moduleId[Module ID]:module_id:' \
                '*:' && return
            ;;
        help)
            _describe 'peer commands' peer_commands
            _describe 'network commands' network_commands
            _describe 'group commands' group_commands
            _describe 'query commands' query_commands
            _describe 'module commands' module_commands
            _describe 'system commands' system_commands
            return
            ;;
    esac

    # Global argument definitions
    _arguments -s \
        '--serverAddress[Server address to connect to]:address:(localhost 127.0.0.1 \:\:1)' \
        '--serverPort[Server JSON-RPC port]:port:(8080 9000 8089)' \
        '--command[Command to execute]:command:->cmd' \
        '--verbose[Enable verbose output]' \
        '--json[Output as JSON]' \
        '--quiet[Suppress non-essential output]' \
        '--format[Output format]:format:->fmt' \
        '--color[Enable colored output]' \
        '--completions[Generate shell completions]:shell:->shell' \
        '--logFile[Path to log file]:file:_files' \
        '--logLevel[Logging level]:level:->level' \
        '--help[Show usage information]' \
        '--version[Show version]'

    case $state in
        cmd)
            _describe -t peer-commands 'peer commands' peer_commands
            _describe -t network-commands 'network commands' network_commands
            _describe -t group-commands 'group commands' group_commands
            _describe -t query-commands 'query commands' query_commands
            _describe -t module-commands 'module commands' module_commands
            _describe -t system-commands 'system commands' system_commands
            ;;
        fmt)
            compadd -- $formats
            ;;
        shell)
            compadd -- $shells
            ;;
        level)
            compadd -- $log_levels
            ;;
    esac
}

_alpc "$@"

import os

/// Centralized logging with categorized loggers for structured diagnostics.
struct AppLogger {
    private static let subsystem = "com.sonoranpub.AlpineApp"

    static let network = Logger(subsystem: subsystem, category: "network")
    static let auth = Logger(subsystem: subsystem, category: "auth")
    static let broadcast = Logger(subsystem: subsystem, category: "broadcast")
    static let discovery = Logger(subsystem: subsystem, category: "discovery")
    static let transport = Logger(subsystem: subsystem, category: "transport")
    static let ui = Logger(subsystem: subsystem, category: "ui")
    static let general = Logger(subsystem: subsystem, category: "general")
}

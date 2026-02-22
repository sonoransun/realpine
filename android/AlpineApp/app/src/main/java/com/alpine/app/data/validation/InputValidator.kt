package com.alpine.app.data.validation

object InputValidator {
    private val IP_REGEX = Regex("""^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})$""")
    private val HOSTNAME_REGEX = Regex("""^[a-zA-Z0-9]([a-zA-Z0-9\-]{0,61}[a-zA-Z0-9])?(\.[a-zA-Z0-9]([a-zA-Z0-9\-]{0,61}[a-zA-Z0-9])?)*$""")
    private val FINGERPRINT_REGEX = Regex("""^(sha256/)?[A-Za-z0-9+/]{43}=$""")

    fun isValidHost(host: String): Boolean {
        if (host.isBlank() || host.length > 253) return false
        return isValidIpAddress(host) || HOSTNAME_REGEX.matches(host)
    }

    fun isValidPort(port: String): Boolean {
        val portNum = port.toIntOrNull() ?: return false
        return portNum in 1..65535
    }

    fun isValidIpAddress(ip: String): Boolean {
        val match = IP_REGEX.matchEntire(ip) ?: return false
        return match.groupValues.drop(1).all {
            val n = it.toIntOrNull() ?: return false
            n in 0..255
        }
    }

    fun isValidFingerprint(fp: String): Boolean {
        if (fp.isBlank()) return true
        return FINGERPRINT_REGEX.matches(fp)
    }

    fun sanitizeQueryString(query: String, maxLength: Int = 1024): String {
        return query.take(maxLength)
    }
}

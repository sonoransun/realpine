package com.alpine.app.data.storage

import android.content.Context
import android.content.SharedPreferences
import androidx.security.crypto.EncryptedSharedPreferences
import androidx.security.crypto.MasterKey

class SecureStorage(private val context: Context) {
    private companion object {
        const val PREFS_NAME = "alpine_secure_prefs"
    }

    private fun getPrefs(): SharedPreferences {
        val masterKey = MasterKey.Builder(context)
            .setKeyScheme(MasterKey.KeyScheme.AES256_GCM)
            .build()
        return EncryptedSharedPreferences.create(
            context,
            PREFS_NAME,
            masterKey,
            EncryptedSharedPreferences.PrefKeyEncryptionScheme.AES256_SIV,
            EncryptedSharedPreferences.PrefValueEncryptionScheme.AES256_GCM
        )
    }

    fun store(key: String, value: String) {
        getPrefs().edit().putString(key, value).apply()
    }

    fun read(key: String): String? {
        return getPrefs().getString(key, null)
    }

    fun remove(key: String) {
        getPrefs().edit().remove(key).apply()
    }
}

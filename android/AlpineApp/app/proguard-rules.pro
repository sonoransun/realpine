# Keep Gson model classes
-keep class com.alpine.app.data.model.** { *; }
-keep class com.alpine.app.data.rpc.JsonRpcException { *; }

# Keep Gson serialized fields
-keepclassmembers class * {
    @com.google.gson.annotations.SerializedName <fields>;
}

# Keep Compose
-dontwarn androidx.compose.**

# Keep Retrofit
-keepattributes Signature
-keepattributes *Annotation*

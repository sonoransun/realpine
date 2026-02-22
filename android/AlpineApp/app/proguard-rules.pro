# Keep kotlinx.serialization
-keepattributes *Annotation*, InnerClasses
-dontnote kotlinx.serialization.AnnotationsKt

-keepclassmembers class kotlinx.serialization.json.** {
    *** Companion;
}
-keepclasseswithmembers class kotlinx.serialization.json.** {
    kotlinx.serialization.KSerializer serializer(...);
}
-keep,includedescriptorclasses class com.alpine.app.**$$serializer { *; }
-keepclassmembers class com.alpine.app.** {
    *** Companion;
}
-keepclasseswithmembers class com.alpine.app.** {
    kotlinx.serialization.KSerializer serializer(...);
}

# Keep Hilt
-keep class dagger.hilt.** { *; }
-keep class javax.inject.** { *; }
-keep class * extends dagger.hilt.android.internal.managers.ViewComponentManager$FragmentContextWrapper { *; }

# Keep JsonRpcException
-keep class com.alpine.app.data.rpc.JsonRpcException { *; }

# Keep Compose
-dontwarn androidx.compose.**

# Keep Retrofit
-keepattributes Signature
-keepattributes *Annotation*

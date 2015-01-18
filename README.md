# Bifröst
WebP Advanced API wrapper for Android

Import 'bifrost' module into your project to use.

JNI code is rebuilt with:
```
cd bifrost/jni/
ndk-build
```

This will place binaries in bifrost/libs and bifrost/obj. You then want to do...

```
mv bifrost/libs/{armeabi,armeabi-v7a,x86} src/main/jniLibs/
```

...as ndk-build places these wrong for Android Studio.

The jni/ folder contains the libwebp source code with a custom JNI wrapper (wrapper/Bifrost.*) and Application.mk added.

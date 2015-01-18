# Bifröst
WebP Advanced API wrapper for Android
by Thor Harald Johansen for Imoji

This wrapper was developed to make encoding and decoding (optionally at reduced scale) of WebP images with alpha transparency possible on any Android device.

Import 'bifrost' module into your project to use.

WebP decode (no scaling):

```
Bitmap bitmap = Bifrost.decode(byteBuffer, 16384, 16384);
```

WebP decode (reduced scale):

```
Bitmap bitmap = Bifrost.decode(byteBuffer, 128, 128);
```

The image is scaled down with the original aspect ratio preserved. For example, if the source image was 1000x2000 pixels, the output would be 64x128 pixels in the example above.

WebP encode:

```
ByteBuffer byteBuffer = Bifrost.encode(bitmap, Bifrost.WEBP_PRESET_PICTURE, 50);
```

This will encode a WebP image at quality factor 50 (the maximum is 100).

Available presets are:

```
Bifrost.WEBP_PRESET_DEFAULT // default preset
Bifrost.WEBP_PRESET_PICTURE // digital picture, like portrait, inner shot
Bifrost.WEBP_PRESET_PHOTO   // outdoor photograph, with natural lighting
Bifrost.WEBP_PRESET_DRAWING // hand or line drawing, with high-contrast details
Bifrost.WEBP_PRESET_ICON    // small-sized colorful images
Bifrost.WEBP_PRESET_TEXT    // text-like
```

Later releases of this wrapper will add convenience methods and expose more of the API.

If desired, JNI code can be rebuilt with:
```
cd bifrost/jni/
ndk-build
```

This will place binaries in bifrost/libs and bifrost/obj. You then want to do...

```
rm -R bifrost/src/main/jniLibs/*
mv bifrost/libs/{armeabi,armeabi-v7a,x86} bifrost/src/main/jniLibs/
```

...as ndk-build places these wrong for Android Studio. The obj/ directory is redundant.

The jni/ folder contains the libwebp source code with a custom JNI wrapper (wrapper/Bifrost.*) and Application.mk added. To use with your own version of libwebp, preserve these custom files, replace the jni/ folder with libwebp, put the files back in their place, and add wrapper/Bifrost.c back to LOCAL_SRC_FILES in the Android.mk file.

/* Created by Thor Harald Johansen for Imoji in 2015 */

#include <stdlib.h>
#include <wchar.h>

#include <android/log.h>
#include <android/bitmap.h>

#include "../src/webp/encode.h"
#include "../src/webp/decode.h"

#include "Bifrost.h"

#define TAG "Bifrost"

int handle_android_bitmap_error(int);

JNIEXPORT jobject JNICALL Java_com_imojiapp_bifrost_Bifrost_decode
        (JNIEnv *env, jclass class, jobject in, jint max_width, jint max_height) {
    
    // Get direct memory access to input Buffer object        
    int data_size = (*env)->GetDirectBufferCapacity(env, in);
    if(data_size == -1) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Buffer is not a direct buffer!");
        return NULL;
    }
    uint8_t *data = (*env)->GetDirectBufferAddress(env, in);
    if(data == NULL) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Unable to get direct buffer address!");
        return NULL;
    }


    // Get original image dimensions
    int orig_width, orig_height;
    if(!WebPGetInfo(data, data_size, &orig_width, &orig_height)) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Unable to get WebP information!");
        return NULL;
    }
    
    WebPDecoderConfig config;
    WebPInitDecoderConfig(&config);

    // Calculate output bitmap size
    if(orig_width > max_width || orig_height > max_height) {
        config.options.use_scaling = 1;
        float image_aspect = orig_width / (float) orig_height;
        float max_aspect = max_width / (float) max_height;
        if(image_aspect > max_aspect) { // Source aspect is wider, fit to width
            config.options.scaled_width = max_width;
            config.options.scaled_height = max_width / image_aspect;
        } else { // Source aspect is narrower, fit to height
            config.options.scaled_width = max_height * image_aspect;
            config.options.scaled_height = max_height;
        }
    } else { // No scaling needed
        config.options.scaled_width = orig_width;
        config.options.scaled_height = orig_height;
    }
    
    // Create Bitmap with ARGB_8888 configuration

    jclass bitmap_class = (*env)->FindClass(env, "android/graphics/Bitmap");
    if(bitmap_class == NULL) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Unable to find Bitmap class!");
        return NULL;
    }

    jstring config_name_string = (*env)->NewStringUTF(env, "ARGB_8888");
    if(config_name_string == NULL) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Unable to construct Bitmap configuration name string!");
        return NULL;
    }
    jclass bitmap_config_class = (*env)->FindClass(env, "android/graphics/Bitmap$Config");
    if(bitmap_config_class == NULL) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Unable to find Bitmap.Config class!");
        return NULL;
    }
    jobject bitmap_config = (*env)->CallStaticObjectMethod(env, bitmap_config_class,
        (*env)->GetStaticMethodID(env, bitmap_config_class, "valueOf",
        "(Ljava/lang/String;)Landroid/graphics/Bitmap$Config;"), config_name_string);
    if(bitmap_config == NULL) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Bitmap.Config.valueOf(\"ARGB_8888\") returned NULL!");
        return NULL;
    }

    jmethodID create_bitmap_method = (*env)->GetStaticMethodID(env, bitmap_class, "createBitmap", "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");
    if(create_bitmap_method == NULL) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Unable to find Bitmap Bitmap.createBitmap(int, int, Bitmap.Config)!");
        return NULL;
    }
    jobject bitmap = (*env)->CallStaticObjectMethod(env, bitmap_class, create_bitmap_method,
        config.options.scaled_width, config.options.scaled_height, bitmap_config);
    if(bitmap == NULL) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Bitmap.createBitmap() returned NULL!");
        return NULL;
    }

    // Get information about Bitmap pixel buffer
    AndroidBitmapInfo bitmap_info;
    int android_bitmap_result = AndroidBitmap_getInfo(env, bitmap, &bitmap_info);
    if(handle_android_bitmap_error(android_bitmap_result)) {
        return NULL;
    }

    // Configure for ARGB output to external buffer
    config.output.colorspace = MODE_RGBA; // Android calls it "ARGB" but it's actually RGBA!
    config.output.is_external_memory = 1;
    
    // Configure to output to match stride and size of Bitmap
    config.output.u.RGBA.stride = bitmap_info.stride;
    config.output.u.RGBA.size = bitmap_info.height * bitmap_info.stride;

    // Lock Bitmap pixel buffer and let WebP access it
    android_bitmap_result = AndroidBitmap_lockPixels(env, bitmap, (void **) &config.output.u.RGBA.rgba);
    if(handle_android_bitmap_error(android_bitmap_result)) {
        return NULL;
    }

    // Perform actual decode
    VP8StatusCode webp_status = WebPDecode(data, data_size, &config);
    switch(webp_status) {
    case VP8_STATUS_OK:
        break;

    case VP8_STATUS_OUT_OF_MEMORY:
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Out of memory during WebP decode!");
        return NULL;

    case VP8_STATUS_INVALID_PARAM:
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Invalid parameter passed to WebPDecode()!");
        return NULL;

    case VP8_STATUS_BITSTREAM_ERROR:
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Bitstream error during WebP decode!");
        return NULL;

    case VP8_STATUS_UNSUPPORTED_FEATURE:
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Unsupported feature during WebP decode!");
        return NULL;

    case VP8_STATUS_SUSPENDED:
        __android_log_print(ANDROID_LOG_ERROR, TAG, "WebP decode is suspended (not supposed to happen)!");
        return NULL;

    case VP8_STATUS_USER_ABORT:
        __android_log_print(ANDROID_LOG_ERROR, TAG, "User abort during WebP decode!");
        return NULL;

    case VP8_STATUS_NOT_ENOUGH_DATA:
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Not enough data during WebP decode!");
        return NULL;
    }

    // Unlock Bitmap pixel buffer
    android_bitmap_result = AndroidBitmap_unlockPixels(env, bitmap);
    if(handle_android_bitmap_error(android_bitmap_result)) {
        return NULL;
    }

    return bitmap;
}

JNIEXPORT jobject JNICALL Java_com_imojiapp_bifrost_Bifrost_encode
        (JNIEnv *env, jclass class, jobject bitmap, jint preset, jfloat quality) {
    WebPConfig config;
    if (!WebPConfigPreset(&config, preset, quality)) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "WebP configuration preset error!");
        return NULL;
    }
    if(!WebPValidateConfig(&config)) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "WebP configuration value range error!");
        return NULL;
    }

    // Get information about Bitmap pixel buffer
    AndroidBitmapInfo bitmap_info;
    int android_bitmap_result = AndroidBitmap_getInfo(env, bitmap, &bitmap_info);
    if(handle_android_bitmap_error(android_bitmap_result)) {
        return NULL;
    }

    WebPPicture pic;
    if(!WebPPictureInit(&pic)) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "WebP picture initialization error!");
        return NULL;
    }
    pic.width = bitmap_info.width;
    pic.height = bitmap_info.height;

    uint8_t *pixels = NULL;

    // Lock Bitmap pixel buffer and let WebP access it
    android_bitmap_result = AndroidBitmap_lockPixels(env, bitmap, (void **) &pixels);
    if(handle_android_bitmap_error(android_bitmap_result)) {
        return NULL;
    }

    if(!WebPPictureImportRGBA(&pic, pixels, bitmap_info.stride)) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Error during WebP picture import (likely out of memory)!");
        return NULL;
    }

    // Unlock Bitmap pixel buffer
    android_bitmap_result = AndroidBitmap_unlockPixels(env, bitmap);
    if(handle_android_bitmap_error(android_bitmap_result)) {
        return NULL;
    }

    WebPMemoryWriter writer;
    WebPMemoryWriterInit(&writer);
    pic.writer = WebPMemoryWrite;
    pic.custom_ptr = &writer;

    int webp_success = WebPEncode(&config, &pic);
    WebPPictureFree(&pic);
    if(!webp_success) {
        switch(pic.error_code) {
        case VP8_ENC_OK:
        case VP8_ENC_ERROR_LAST:
            break;

        case VP8_ENC_ERROR_OUT_OF_MEMORY:
            __android_log_print(ANDROID_LOG_ERROR, TAG, "Out of memory allocating objects during WebP encode!");
            return NULL;

        case VP8_ENC_ERROR_BITSTREAM_OUT_OF_MEMORY:
            __android_log_print(ANDROID_LOG_ERROR, TAG, "Out of memory flushing bits during WebP encode!");
            return NULL;

        case VP8_ENC_ERROR_NULL_PARAMETER:
            __android_log_print(ANDROID_LOG_ERROR, TAG, "Unable to encode WebP due to NULL parameter!");
            return NULL;

        case VP8_ENC_ERROR_INVALID_CONFIGURATION:
            __android_log_print(ANDROID_LOG_ERROR, TAG, "Unable to encode WebP due to invalid configuration!");
            return NULL;

        case VP8_ENC_ERROR_BAD_DIMENSION:
            __android_log_print(ANDROID_LOG_ERROR, TAG, "Unable to encode WebP due to invalid image dimensions!!");
            return NULL;

        case VP8_ENC_ERROR_PARTITION0_OVERFLOW:
            __android_log_print(ANDROID_LOG_ERROR, TAG, "Unable to encode WebP due to partition being >512k!");
            return NULL;

        case VP8_ENC_ERROR_PARTITION_OVERFLOW:
            __android_log_print(ANDROID_LOG_ERROR, TAG, "Unable to encode WebP due to partition being >16M!");
            return NULL;

        case VP8_ENC_ERROR_BAD_WRITE:
            __android_log_print(ANDROID_LOG_ERROR, TAG, "Unable to flush bytes during WebP encode!");
            return NULL;

        case VP8_ENC_ERROR_FILE_TOO_BIG:
            __android_log_print(ANDROID_LOG_ERROR, TAG, "Unable to encode WebP due to file being >4G!");
            return NULL;

        case VP8_ENC_ERROR_USER_ABORT:
            __android_log_print(ANDROID_LOG_ERROR, TAG, "Unable to encode WebP due to user abort!");
            return NULL;
        }
    }

    if(writer.size < writer.max_size) {
        writer.mem = realloc(writer.mem, writer.size);
    }

    jobject buffer = (*env)->NewDirectByteBuffer(env, writer.mem, writer.size);
    if(buffer == NULL) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Unable to create direct ByteBuffer during WebP encode!");
        free(writer.mem);
        return NULL;
    }

    return buffer;
}

JNIEXPORT void JNICALL Java_com_imojiapp_bifrost_Bifrost_releaseBuffer
        (JNIEnv *env, jclass class, jobject buffer) {
	void *address = (*env)->GetDirectBufferAddress(env, buffer);
	free(address);
}

int handle_android_bitmap_error(int result) {
    switch(result) {
    case ANDROID_BITMAP_RESULT_SUCCESS:
        return 0;
    case ANDROID_BITMAP_RESULT_BAD_PARAMETER:
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Bad parameter when getting Android Bitmap info!");
        return 1;
    case ANDROID_BITMAP_RESULT_JNI_EXCEPTION:
        __android_log_print(ANDROID_LOG_ERROR, TAG, "JNI exception when getting Android Bitmap info!");
        return 1;
    case ANDROID_BITMAP_RESULT_ALLOCATION_FAILED:
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Allocation failed when getting Android Bitmap info!");
        return 1;
    default:
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Error %d when getting Android Bitmap info!", result);
        return 1;
    }
}

/* Created by Thor Harald Johansen for Imoji in 2015 */

#include <jni.h>

#ifndef __BIFROST_H__
#define __BIFROST_H__
#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jobject JNICALL Java_com_imojiapp_bifrost_Bifrost_decode
  (JNIEnv *, jclass, jobject, jint, jint);

JNIEXPORT jobject JNICALL Java_com_imojiapp_bifrost_Bifrost_encode
  (JNIEnv *, jclass, jobject, jint, jfloat);

JNIEXPORT void JNICALL Java_com_imojiapp_bifrost_Bifrost_releaseBuffer
  (JNIEnv *env, jclass class, jobject buffer);

#ifdef __cplusplus
}
#endif
#endif

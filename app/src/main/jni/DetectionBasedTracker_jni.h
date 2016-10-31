#include <jni.h>

#ifndef _Included_com_yar_nativefacedetector_DetectionBasedTracker
#define _Included_com_yar_nativefacedetector_DetectionBasedTracker

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jlong JNICALL nativeCreateObject(JNIEnv *, jclass, jstring, jint);

JNIEXPORT void JNICALL nativeDestroyObject(JNIEnv *, jclass, jlong);

JNIEXPORT void JNICALL nativeStart(JNIEnv *, jclass, jlong);

JNIEXPORT void JNICALL nativeStop(JNIEnv *, jclass, jlong);

JNIEXPORT void JNICALL nativeSetFaceSize(JNIEnv *, jclass, jlong, jint);

JNIEXPORT void JNICALL nativeDetect(JNIEnv *, jclass, jlong, jlong, jlong);

#ifdef __cplusplus
}
#endif

static const JNINativeMethod methods[] = {
        {"nativeCreateObject",  "(Ljava/lang/String;I)J", (void *) nativeCreateObject},
        {"nativeDestroyObject", "(J)V",                   (void *) nativeDestroyObject},
        {"nativeStart",         "(J)V",                   (void *) nativeStart},
        {"nativeStop",          "(J)V",                   (void *) nativeStop},
        {"nativeSetFaceSize",   "(JI)V",                  (void *) nativeSetFaceSize},
        {"nativeDetect",        "(JJJ)V",                 (void *) nativeDetect}
};

static const char *CLASS_NAME = "com/yar/nativefacedetector/DetectionBasedTracker";

static int registerNativeMethods(JNIEnv *env, const char *className,
                                 JNINativeMethod* gMethods, int numMethods) {
    jclass clazz;

    clazz = env->FindClass(className);
    if (clazz == nullptr) {
        return JNI_FALSE;
    }
    if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env;
    if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }


    if (registerNativeMethods(env,CLASS_NAME, const_cast<JNINativeMethod*>(methods),
                              sizeof(methods) / sizeof(methods[0])) != JNI_TRUE) {
        return JNI_ERR;
    }

    return JNI_VERSION_1_6;
}

#endif

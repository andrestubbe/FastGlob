#ifndef FASTXXX_H
#define FASTXXX_H

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

// Export declarations (Matches fastglob.def)
JNIEXPORT jobjectArray JNICALL Java_fastglob_FastGLOB_glob(JNIEnv* env, jclass clazz, jstring baseDir, jstring pattern);

#ifdef __cplusplus
}
#endif

#endif // FASTXXX_H

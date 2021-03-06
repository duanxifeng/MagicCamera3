#include <android/log.h>
#include "EGLCore.h"
#include <android/native_window.h>
#include <EGL/eglext.h>


#define LOG_TAG "EGLCore"
#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define ALOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

/**
 * cangwang 2018.12.1
 */
EGLCore::EGLCore():mDisplay(EGL_NO_DISPLAY),mSurface(EGL_NO_SURFACE),mContext(EGL_NO_CONTEXT) {


}

EGLCore::~EGLCore() {
    mDisplay = EGL_NO_DISPLAY;
    mSurface = EGL_NO_SURFACE;
    mContext = EGL_NO_CONTEXT;
}

GLboolean EGLCore::buildContext(ANativeWindow *window) {
    //与本地窗口通信
    mDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (mDisplay == EGL_NO_DISPLAY){
        ALOGE("eglGetDisplay failed: %d",eglGetError());
        return GL_FALSE;
    }

    GLint majorVersion;
    GLint minorVersion;
    //获取支持最低和最高版本
    if (!eglInitialize(mDisplay,&majorVersion,&minorVersion)){
        ALOGE("eglInitialize failed: %d",eglGetError());
        return GL_FALSE;
    }

    EGLConfig config;
    EGLint numConfigs = 0;
    //颜色使用565，读写类型需要egl扩展
    EGLint attribList[] = {
            EGL_RED_SIZE,5,
            EGL_GREEN_SIZE,6,
            EGL_BLUE_SIZE,5,
            EGL_RENDERABLE_TYPE,EGL_OPENGL_ES3_BIT_KHR, //渲染类型，为扩展类型
            EGL_SURFACE_TYPE,EGL_WINDOW_BIT,  //绘图类型，
            EGL_NONE
    };

    //让EGL推荐匹配的EGLConfig
    if(!eglChooseConfig(mDisplay,attribList,&config,1,&numConfigs)){
        ALOGE("eglChooseConfig failed: %d",eglGetError());
        return GL_FALSE;
    }

    //找不到匹配的
    if (numConfigs <1){
        ALOGE("eglChooseConfig get config number less than one");
        return GL_FALSE;
    }

    //创建渲染上下文
    //只使用opengles3
    GLint contextAttrib[] = {EGL_CONTEXT_CLIENT_VERSION,3,EGL_NONE};
    // EGL_NO_CONTEXT表示不向其它的context共享资源
    mContext = eglCreateContext(mDisplay,config,EGL_NO_CONTEXT,contextAttrib);
    if (mContext == EGL_NO_CONTEXT){
        ALOGE("eglCreateContext failed: %d",eglGetError());
        return GL_FALSE;
    }

    EGLint format = 0;
    if (!eglGetConfigAttrib(mDisplay,config,EGL_NATIVE_VISUAL_ID,&format)){
        ALOGE("eglGetConfigAttrib failed: %d",eglGetError());
        return GL_FALSE;
    }
    ANativeWindow_setBuffersGeometry(window,0,0,format);

    //创建On-Screen 渲染区域
    mSurface = eglCreateWindowSurface(mDisplay,config,window,0);
    if (mSurface == EGL_NO_SURFACE){
        ALOGE("eglCreateWindowSurface failed: %d",eglGetError());
        return GL_FALSE;
    }

    //把EGLContext和EGLSurface关联起来
    if (!eglMakeCurrent(mDisplay,mSurface,mSurface,mContext)){
        ALOGE("eglMakeCurrent failed: %d",eglGetError());
        return GL_FALSE;
    }

    ALOGD("buildContext Succeed");
    return GL_TRUE;
}

void EGLCore::swapBuffer() {
    //双缓冲绘图，原来是检测出前台display和后台缓冲的差别的dirty区域，然后再区域替换buffer
    //1）首先计算非dirty区域，然后将非dirty区域数据从上一个buffer拷贝到当前buffer；
    //2）完成buffer内容的填充，然后将previousBuffer指向buffer，同时queue buffer。
    //3）Dequeue一块新的buffer，并等待fence。如果等待超时，就将buffer cancel掉。
    //4）按需重新计算buffer
    //5）Lock buffer，这样就实现page flip，也就是swapbuffer
    eglSwapBuffers(mDisplay,mSurface);
}

void EGLCore::release() {
    eglDestroySurface(mDisplay,mSurface);
    eglMakeCurrent(mDisplay,EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT);
    eglDestroyContext(mDisplay,mContext);
}
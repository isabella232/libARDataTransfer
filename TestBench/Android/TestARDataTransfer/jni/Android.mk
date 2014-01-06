
#http://www.kandroid.org/ndk/docs/ANDROID-MK.html
#http://stackoverflow.com/questions/9870435/how-can-i-link-prebuilt-shared-library-to-android-ndk-project
#
# ln -s ./jni/c ~/FreeFlight3/libARDataTransfer/JNI/c
# ~/dev/sdk/android-ndk-r8e/ndk-build NDK_DEBUG=1 (ndk-r9, r9b dosen't work for debugging)
# ~/dev/sdk/android-ndk-r9b/ndk-gdb --launch=com.example.tstdata.MainActivity --verbose

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libcurl
LOCAL_SRC_FILES := /home/smonat/FreeFlight3/ARBuildUtils/Targets/Android/Install/armeabi/lib/libcurl.so
LOCAL_EXPORT_C_INCLUDES := /home/smonat/FreeFlight3/ARBuildUtils/Targets/Android/Install/armeabi/include
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libarsal_dbg
LOCAL_SRC_FILES := /home/smonat/FreeFlight3/ARBuildUtils/Targets/Android/Install/armeabi/lib/libarsal_dbg.so
LOCAL_EXPORT_C_INCLUDES := /home/smonat/FreeFlight3/ARBuildUtils/Targets/Android/Install/armeabi/include
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libarsal_android_dbg
LOCAL_SRC_FILES := /home/smonat/FreeFlight3/ARBuildUtils/Targets/Android/Install/armeabi/lib/libarsal_android_dbg.so
LOCAL_EXPORT_C_INCLUDES := /home/smonat/FreeFlight3/ARBuildUtils/Targets/Android/Install/armeabi/include
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libarutils_dbg
LOCAL_SRC_FILES := /home/smonat/FreeFlight3/ARBuildUtils/Targets/Android/Install/armeabi/lib/libarutils_dbg.so
LOCAL_EXPORT_C_INCLUDES := /home/smonat/FreeFlight3/ARBuildUtils/Targets/Android/Install/armeabi/include
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libarutils_android_dbg
LOCAL_SRC_FILES := /home/smonat/FreeFlight3/ARBuildUtils/Targets/Android/Install/armeabi/lib/libarutils_android_dbg.so
LOCAL_EXPORT_C_INCLUDES := /home/smonat/FreeFlight3/ARBuildUtils/Targets/Android/Install/armeabi/include
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libardatatransfer_dbg
LOCAL_SRC_FILES := /home/smonat/FreeFlight3/ARBuildUtils/Targets/Android/Install/armeabi/lib/libardatatransfer_dbg.so
LOCAL_EXPORT_C_INCLUDES := /home/smonat/FreeFlight3/ARBuildUtils/Targets/Android/Install/armeabi/include
include $(PREBUILT_SHARED_LIBRARY)



include $(CLEAR_VARS)
LOCAL_C_INCLUDES := /home/smonat/FreeFlight3/ARBuildUtils/Targets/Android/Install/armeabi/include
LOCAL_MODULE    := libardatatransfer_android_dbg
#LOCAL_CFLAGS	:= -Wall -Werror -g
LOCAL_CFLAGS	:= -Wall -Werror
LOCAL_SRC_FILES := c/ARDATATRANSFER_JNI_Manager.c \
					c/ARDATATRANSFER_JNI_DataDownloader.c \
					c/ARDATATRANSFER_JNI_MediasDownloader.c
#LOCAL_SRC_FILES := /home/smonat/FreeFlight3/ARBuildUtils/Targets/Android/Install/armeabi/lib/libardatatransfer_android_dbg.so
LOCAL_SHARED_LIBRARIES := libcurl libarsal_dbg libarsal_android_dbg libarutils_dbg libarutils_android_dbg libardatatransfer_dbg


include $(BUILD_SHARED_LIBRARY)




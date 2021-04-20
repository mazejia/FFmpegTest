LOCAL_PATH := $(call my-dir)

# FFmpeg 库
include $(CLEAR_VARS)
LOCAL_MODULE := ffmpeg
LOCAL_SRC_FILES := $(LOCAL_PATH)/libs/$(TARGET_ARCH_ABI)/libffmpeg.so
include $(PREBUILT_SHARED_LIBRARY)

# C文件
include $(CLEAR_VARS)
LOCAL_MODULE := ffmpeg_test
LOCAL_SRC_FILES := ffmpeg_test.cpp
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include/ffmpeg

LOCAL_LDLIBS := -llog -landroid
LOCAL_SHARED_LIBRARIES := ffmpeg
include $(BUILD_SHARED_LIBRARY)
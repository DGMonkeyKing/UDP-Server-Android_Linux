LOCAL_PATH := $(my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := servUDPmod.c

LOCAL_SHARED_LIBRARIES := 

LOCAL_CFLAGS :=

LOCAL_MODULE := udp-android

include $(BUILD_EXECUTABLE)

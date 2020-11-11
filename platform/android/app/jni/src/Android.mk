LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := main

SDL_PATH := ../SDL
SDL_IMAGE_PATH := ../SDL_image
SDL_MIXER_PATH := ../SDL_mixer
SDL_TTF_PATH := ../SDL_ttf
YAML_PATH := ../yaml
SRC_PATH := ../../../../../

LOCAL_CPPFLAGS := -DSOSAGE_DATA_FOLDER=\"data/\" -std=c++17 \
  -frtti \
  -DSOSAGE_LINKED_WITH_SDL_MIXER \
  -DSOSAGE_CFG_USE_THREADS \
	-O3 -DNDEBUG

LOCAL_C_INCLUDES := \
  $(LOCAL_PATH)/$(SRC_PATH)/include \
  $(LOCAL_PATH)/$(SDL_PATH)/include \
  $(LOCAL_PATH)/$(SDL_IMAGE_PATH)/include \
  $(LOCAL_PATH)/$(SDL_MIXER_PATH)/include \
  $(LOCAL_PATH)/$(SDL_TTF_PATH)/include \
  $(LOCAL_PATH)/$(YAML_PATH)/include \

LOCAL_SRC_FILES := \
  $(wildcard $(LOCAL_PATH)/$(SRC_PATH)/src/*.cpp) \
  $(wildcard $(LOCAL_PATH)/$(SRC_PATH)/src/*/*.cpp) \
  $(wildcard $(LOCAL_PATH)/$(SRC_PATH)/src/*/*/*.cpp)


LOCAL_SHARED_LIBRARIES := SDL2 SDL2_image SDL2_mixer SDL2_ttf yaml

LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -llog

include $(BUILD_SHARED_LIBRARY)

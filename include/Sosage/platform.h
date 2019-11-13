#ifndef SOSAGE_PLATFORM_H
#define SOSAGE_PLATFORM_H

#if defined(__ANDROID__)
#  define SOSAGE_ANDROID
#elif defined(__APPLE__)
#  define SOSAGE_MAC
#elif defined(_WIN32)
#  define SOSAGE_WINDOWS
#elif defined(__linux__)
#  define SOSAGE_LINUX
#endif

#endif SOSAGE_PLATFORM_H

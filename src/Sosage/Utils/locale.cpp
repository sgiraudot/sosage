/*
  [src/Sosage/Utils/locale.cpp]
  Accessing locale info on all platforms.

  =====================================================================

  This file is part of SOSAGE.

  SOSAGE is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  SOSAGE is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with SOSAGE.  If not, see <https://www.gnu.org/licenses/>.

  =====================================================================

  Author(s): Simon Giraudot <sosage@ptilouk.net>
*/

#include <Sosage/Config/platform.h>
#include <Sosage/Utils/locale.h>

#if defined(SOSAGE_ANDROID)
#  include <SDL.h>
#  include <jni.h>
#elif defined(SOSAGE_WINDOWS)
#  include <wchar.h>
#  include <winnls.h>
#elif defined(SOSAGE_MAC)
#  include <CoreFoundation/CoreFoundation.h>
#else
#  include <locale>
#endif


namespace Sosage
{

std::string get_locale()
{
#if defined(SOSAGE_ANDROID)
  JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
  jobject activity = (jobject)SDL_AndroidGetActivity();
  jclass jni_class(env->GetObjectClass(activity));
  jmethodID method_id = env->GetStaticMethodID(jni_class, "getLocale", "()Ljava/lang/String;");
  jstring jstr = (jstring) env->CallStaticObjectMethod(jni_class, method_id);
  const char *str = env->GetStringUTFChars(jstr, 0);
  std::string out (str);
  env->ReleaseStringUTFChars(jstr, str);
  env->DeleteLocalRef(jstr);
  env->DeleteLocalRef(jni_class);
  return out;
#elif defined(SOSAGE_WINDOWS)
  wchar_t name[LOCALE_NAME_MAX_LENGTH];
  int l = GetUserDefaultLocaleName(name, LOCALE_NAME_MAX_LENGTH);
  if (l == 0)
    return "";
  std::wstring ws (name, name + l);
  return std::string(ws.begin(), ws.end());
#elif defined(SOSAGE_MAC)
  CFLocaleRef cflocale = CFLocaleCopyCurrent();
  CFStringRef language = (CFStringRef)CFLocaleGetValue(cflocale, kCFLocaleLanguageCode);
  CFStringRef country = (CFStringRef)CFLocaleGetValue(cflocale, kCFLocaleCountryCode);
  char lstr[256], cstr[256];
  CFStringGetCString(language, lstr, 256, kCFStringEncodingUTF8);
  CFStringGetCString(country, cstr, 256, kCFStringEncodingUTF8);
  CFRelease(cflocale);
  return std::string(lstr) + "_" + std::string(cstr);
#else
  try // std::locale might throw runtime error if no locale declared
  {
    return std::locale("").name();
  }
  catch (std::runtime_error&)
  {
  }
  return "";
#endif
}

} // namespace Sosage

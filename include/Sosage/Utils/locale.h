/*
  [include/Sosage/Utils/locale.h]
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

#ifndef SOSAGE_UTILS_LOCALE_H
#define SOSAGE_UTILS_LOCALE_H

#include <Sosage/Config/platform.h>
#ifdef SOSAGE_ANDROID
#include <jni.h>
#endif

#include <locale>

namespace Sosage
{

std::string get_locale()
{
#ifdef SOSAGE_ANDROID
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

#endif // SOSAGE_UTILS_LOCALE_H

#ifndef SOSAGE_THIRD_PARTY_CONFIG_H
#define SOSAGE_THIRD_PARTY_CONFIG_H

#if defined(SOSAGE_LINKED_WITH_SDL)
#include <Sosage/Third_party/SDL.h>
#include <Sosage/Third_party/SDL_events.h>
namespace Sosage
{
typedef Third_party::SDL Graphic_core;
typedef Third_party::SDL_events Input_core;
}
#else
#include <Sosage/Models/Graphic_core.h>
#include <Sosage/Models/Input_core.h>
#include <Sosage/Models/Sound_core.h>
namespace Sosage
{
typedef Models::Graphic_core Graphic_core;
typedef Models::Input_core Input_core;
}
#endif

#if defined(SOSAGE_LINKED_WITH_SDL_MIXER)
#include <Sosage/Third_party/SDL_mixer.h>
namespace Sosage
{
typedef Third_party::SDL_mixer Sound_core;
}
#else
#include <Sosage/Models/Sound_core.h>
namespace Sosage
{
typedef Models::Sound_core Sound_core;
}
#endif

#endif // SOSAGE_THIRD_PARTY_CONFIG_H

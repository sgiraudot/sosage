#include <Sosage/Third_party/SDL.h>

#ifdef SOSAGE_LINKED_WITH_SDL

namespace Sosage::Third_party
{

SDL::SDL (const std::string& game_name,
                               int width, int height,
                               bool fullscreen)
{
  if (SDL_Init(SDL_INIT_VIDEO) == -1)
    error ("Cannot initialize SDL");
	
  if (TTF_Init() == -1)
    error ("Cannot initialize SDL TTF");

  SDL_WM_SetCaption (game_name.c_str(), NULL);
  
  if (fullscreen)
    m_window = SDL_SetVideoMode (width, height, 32,
                                 SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_FULLSCREEN);
  else
    m_window = SDL_SetVideoMode (width, height, 32,
                                 SDL_HWSURFACE | SDL_DOUBLEBUF);

}

SDL::~SDL ()
{
  SDL_Quit ();
  TTF_Quit ();
}

void SDL::begin ()
{
  SDL_FillRect (m_window, NULL, SDL_MapRGB (m_window->format, 0, 0, 0));
}

void SDL::draw (const SDL::Image& image,
                                     int x, int y)
{
  SDL_Rect position;
  position.x = x;
  position.y = y;

  SDL_BlitSurface (image.get(), NULL, m_window, &position);
}

void SDL::draw_line (const int xa, const int ya, const int xb, const int yb)
{
  lineRGBA (m_window, xa, ya, xb, yb, 0, 0, 255, 255);
}

void SDL::end ()
{
  SDL_Flip (m_window);
}

} // namespace Sosage::Third_party

#endif

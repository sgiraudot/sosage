#include <Sosage/Third_party/SDL.h>

namespace Sosage::Third_party
{

SDL_Window* SDL::m_window = nullptr;
SDL_Renderer* SDL::m_renderer = nullptr;

SDL::SDL (const std::string& game_name,
          int width, int height,
          bool fullscreen)
{
  int init = SDL_Init(SDL_INIT_VIDEO);
  check (init != -1, "Cannot initialize SDL");

  // char *base_path = SDL_GetBasePath();
  // if (base_path)
  //   std::cerr << "Base path = " << base_path << std::endl;
  // char *pref_path = SDL_GetPrefPath("Ptilouk", "Sosage");
  // if (pref_path)
  //   std::cerr << "Base path = " << pref_path << std::endl;

  init = TTF_Init();
  check (init != -1, "Cannot initialize SDL TTF");

  m_window = SDL_CreateWindow (game_name.c_str(),
                               SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED,
                               width, height,
                               SDL_WINDOW_RESIZABLE | (fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0));
  check (m_window != nullptr, "Cannot create SDL Window");
  
  m_renderer = SDL_CreateRenderer (m_window, -1, 0);
  check (m_renderer != nullptr, "Cannot create SDL Renderer");

  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");
  SDL_RenderSetLogicalSize(m_renderer, 1920, 1080);
}

SDL::~SDL ()
{
  TTF_Quit ();
  
  SDL_DestroyRenderer (m_renderer);
  SDL_DestroyWindow (m_window);
//  SDL_FreeSurface (m_cursor_surf);
//  SDL_FreeCursor (m_cursor);
  SDL_Quit ();
}

void SDL::set_cursor (const std::string& file_name)
{
  // m_cursor_surf = IMG_Load (file_name.c_str());
  // check (m_cursor_surf != nullptr, "Cannot create image from " + file_name);
  
  // m_cursor = SDL_CreateColorCursor(m_cursor_surf,
  //                                  m_cursor_surf->w / 2, m_cursor_surf->h / 2);
  // check (m_cursor != nullptr, "Cannot create cursor from " + file_name);
  
  // SDL_SetCursor(m_cursor);
}

void SDL::begin ()
{
  SDL_SetRenderDrawColor (m_renderer, 0, 0, 0, 255);
  SDL_RenderClear (m_renderer);
}

void SDL::draw (const Image& image,
                const int xsource, const int ysource,
                const int wsource, const int hsource,
                const int xtarget, const int ytarget,
                const int wtarget, const int htarget)
{
  SDL_Rect source;
  source.x = xsource;
  source.y = ysource;
  source.w = wsource;
  source.h = hsource;
  
  SDL_Rect target;
  target.x = xtarget;
  target.y = ytarget;
  target.w = wtarget;
  target.h = htarget;
  
  SDL_RenderCopy(m_renderer, image.texture, &source, &target);
}

void SDL::draw_line (const int xa, const int ya, const int xb, const int yb)
{
//  lineRGBA (m_window, xa, ya, xb, yb, 0, 0, 255, 255);
}

void SDL::end ()
{
  SDL_RenderPresent (m_renderer);
}

} // namespace Sosage::Third_party


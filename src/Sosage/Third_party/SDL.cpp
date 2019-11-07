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

  init = TTF_Init();
  check (init != -1, "Cannot initialize SDL TTF");

  m_window = SDL_CreateWindow (game_name.c_str(),
                               SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED,
                               width, height,
                               SDL_WINDOW_RESIZABLE | (fullscreen ? SDL_WINDOW_FULLSCREEN : 0));
  check (m_window != nullptr, "Cannot create SDL Window");
  
  m_renderer = SDL_CreateRenderer (m_window, -1, 0);
  check (m_renderer != nullptr, "Cannot create SDL Renderer");

  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");
  SDL_RenderSetLogicalSize(m_renderer, 1920, 1080);
}

SDL::~SDL ()
{
  SDL_DestroyRenderer (m_renderer);
  SDL_DestroyWindow (m_window);
  TTF_Quit ();
  SDL_Quit ();
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
  
  SDL_RenderCopy(m_renderer, image.first, &source, &target);
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


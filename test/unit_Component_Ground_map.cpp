#include <Sosage/Component/Ground_map.h>
#include <Sosage/Component/Image.h>
#include <Sosage/Config/config.h>
#include <Sosage/Core/Graphic.h>
#include <Sosage/Utils/time.h>
#include <Sosage/Utils/random.h>

#define NB_RANDOM_TESTS_PER_PIXEL 1
#define DISPLAY_RATIO 0.75

int main()
{
  int seed = time(NULL);
  std::cerr << "Random seed = " << seed << std::endl;
  srand(seed);

  Sosage::Core::Graphic graphic;
  int width = DISPLAY_RATIO * Sosage::Config::world_width,
      height = DISPLAY_RATIO * Sosage::Config::world_height;
  graphic.init(width, height, false);

  std::string ground_maps = SOSAGE_GROUND_MAPS;
  std::size_t begin = 0;
  while (begin < ground_maps.size())
  {
    std::size_t end = ground_maps.find(';', begin+1);
    if (end == std::string::npos)
      end = ground_maps.size();

    std::string filename (ground_maps.begin() + begin, ground_maps.begin() + end);
    std::cerr << "[TESTING " << filename << "]" << std::endl;
    begin = end + 1;

    Sosage::Component::Ground_map ground_map("", filename, 1000, 0, [](){});
    Sosage::Component::Image image("", filename);

    Sosage::Clock clock;
    double latest = clock.time();
    for (std::size_t x = 0; x < Sosage::Config::world_width; ++ x)
    {
      for (std::size_t y = 0; y < Sosage::Config::world_height; ++ y)
      {
        int xsource = x;
        int ysource = y;
        for (std::size_t t = 0; t < NB_RANDOM_TESTS_PER_PIXEL; ++ t)
        {
          bool display = false;
          bool okay = true;

          int xtarget = Sosage::random_int(0, Sosage::Config::world_width);
          int ytarget = Sosage::random_int(0, Sosage::Config::world_height);
          std::vector<Sosage::Point> path;
          try
          {
            ground_map.find_path(Sosage::Point(xsource, ysource),
                                 Sosage::Point(xtarget, ytarget),
                                 path);
          }
          catch (std::runtime_error& error)
          {
            std::cerr << std::endl << " * Computing path from " << xsource << "x" << ysource
                      << " to " << xtarget << "x" << ytarget << " crashed: "
                      << error.what() << std::endl;
            display = true;
            okay = false;
          }

          clock.update();
          if (clock.time() - latest > 1.)
          {
            display = true;
            latest = clock.time();
          }

          if (display)
          {
            graphic.begin(0, 0);
            graphic.draw(image.core(),
                         0, 0, Sosage::Config::world_width, Sosage::Config::world_height,
                         0, 0, DISPLAY_RATIO * Sosage::Config::world_width, DISPLAY_RATIO * Sosage::Config::world_height);

#if 0
            ground_map.for_each_vertex
              ([&](const Sosage::Point& point)
               {
                 graphic.draw_square (DISPLAY_RATIO * point.x(), DISPLAY_RATIO * point.Y(), 10);
               });

            ground_map.for_each_edge
              ([&](const Sosage::Point& source, const Sosage::Point& target, bool border)
               {
                 graphic.draw_line (DISPLAY_RATIO * source.x(), DISPLAY_RATIO * source.Y(),
                                    DISPLAY_RATIO * target.x(), DISPLAY_RATIO * target.Y(),
                                    (border ? 255 : 0), 0, (border ? 0 : 255));
               });
#endif
            graphic.draw_line(DISPLAY_RATIO * xsource, DISPLAY_RATIO * ysource,
                              DISPLAY_RATIO * xtarget, DISPLAY_RATIO * ytarget);
            if (okay)
              for (std::size_t i = 0; i < path.size() - 1; ++ i)
                graphic.draw_line (DISPLAY_RATIO * path[i].x(), DISPLAY_RATIO * path[i].y(),
                                   DISPLAY_RATIO * path[i+1].x(), DISPLAY_RATIO * path[i+1].y(),
                                   128);


            graphic.end();

          }
        }
      }
      std::cerr << "\r" << 100. * (x+1) / double(Sosage::Config::world_width) << "% done";
    }
    std::cerr << std::endl;
  }

  return EXIT_SUCCESS;
}

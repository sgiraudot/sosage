#include <Sosage/Component/Action.h>
#include <Sosage/Component/Event.h>
#include <Sosage/Component/Inventory.h>
#include <Sosage/Component/Position.h>
#include <Sosage/Component/Text.h>
#include <Sosage/System/Interface.h>

namespace Sosage::System
{

Interface::Interface (Content& content)
  : m_content (content)
  , m_layout (INIT)
  , m_action_min_height(50)
  , m_interface_min_height(150)
{

}

void Interface::main()
{
  if (m_content.request<Component::Event>("window:rescaled"))
    update_responsive();

  Component::Position_handle cursor
    = m_content.request<Component::Position>("cursor:position");
  if (cursor)
    detect_collision (cursor);

  Component::Position_handle clicked
    = m_content.request<Component::Position>("cursor:clicked");
  if (clicked && m_collision)
  {
    if (m_collision->entity().find("verb_") == 0)
    {
      m_content.get<Component::Variable> ("chosen_verb:text")
        ->set(m_content.get<Component::Text>(m_collision->entity() + ":text"));
      m_content.remove("cursor:clicked");
      m_content.set<Component::Event>("game:verb_clicked");
    }
    else
    {
      std::string verb
        = m_content.get<Component::Text> ("chosen_verb:text")->entity();
      verb = std::string (verb.begin() + 5, verb.end());

      if (verb == "goto")
      {
        Component::State_handle state
          = m_content.request<Component::State>(m_collision->entity() + ":state");
        if (state && (state->value() == "inventory"))
        {
          Component::Action_handle action
            = m_content.request<Component::Action> (m_collision->entity() + ":look");
         if (action)
           m_content.set<Component::Variable>("character:action", action);
         m_content.remove("cursor:clicked");
        }
        else
          m_content.set<Component::Variable>("cursor:target", m_collision);
      }
      else
      {
        Component::Action_handle action
          = m_content.request<Component::Action> (m_collision->entity() + ":" + verb);
        if (action)
          m_content.set<Component::Variable>("character:action", action);

        m_content.remove("cursor:clicked");
      }
      
      m_content.get<Component::Variable> ("chosen_verb:text")
        ->set(m_content.get<Component::Text>("verb_goto:text"));
    }
  }

  update_action();
  update_inventory();
  
}

void Interface::init()
{
  Component::Font_handle interface_font = m_content.get<Component::Font> ("interface:font");
  std::string color_str = m_content.get<Component::Text> ("interface:color")->value();

  for (const auto& verb : { std::make_pair (std::string("open"), std::string("Ouvrir")),
        std::make_pair (std::string("close"), std::string("Fermer")),
        std::make_pair (std::string("give"), std::string("Donner")),
        std::make_pair (std::string("take"), std::string("Prendre")),
        std::make_pair (std::string("look"), std::string("Regarder")),
        std::make_pair (std::string("talk"), std::string("Parler")),
        std::make_pair (std::string("use"), std::string("Utiliser")),
        std::make_pair (std::string("move"), std::string("Déplacer")) })
  {
    m_content.set<Component::Text> ("verb_" + verb.first + ":text", verb.second);
    Component::Image_handle verb_img
      = m_content.set<Component::Image> ("verb_" + verb.first + ":image",
                                         interface_font, color_str, verb.second);
    m_content.set<Component::Position> ("verb_" + verb.first + ":position",
                                        Point(0,0));
    m_verbs.push_back (verb_img);
    verb_img->set_relative_origin(0.5, 0.5);
  }

  // TODO: Inventory arrows

  Component::Text_handle
    verb_goto = m_content.set<Component::Text> ("verb_goto:text", "Aller vers");
  m_content.set<Component::Variable>("chosen_verb:text", verb_goto);

  // Create pause screen
  m_content.set<Component::Boolean>("game:paused", false);
  
  // Create lock variable
  m_content.set<Component::Boolean>("game:locked", false);

  m_content.set<Component::Position>("pause_screen:position", Point(0, 0));

  update_responsive();
}

void Interface::update_responsive()
{
  int world_width = config().world_width;
  int world_height = config().world_height;
  int window_width = config().window_width;
  int window_height = config().window_height;

  int aspect_ratio = int(100. * window_width / double(window_height));

  if (aspect_ratio >= 200)
  {
    if (m_layout == WIDESCREEN)
      return;
    m_layout = WIDESCREEN;
    
    interface_widescreen();
    vertical_layout();
  }
  else
  {
    if (aspect_ratio >= 115)
    {
      if (m_layout == STANDARD)
        return;
      m_layout = STANDARD;
      
      interface_standard();
    }
    else if (aspect_ratio >= 75)
    {
      if (m_layout == SQUARE)
        return;
      m_layout = SQUARE;
      
      interface_square();
    }
    else
    {
      if (m_layout == PORTRAIT)
        return;
      m_layout = PORTRAIT;
      
      interface_portrait();
    }
    
    horizontal_layout();
  }

  update_pause_screen();
}

void Interface::interface_widescreen()
{
  int world_width = 1920;
  int world_height = 1000;
  m_action_height = 50;
  int interface_height = 525;
  int interface_width = 180;

  m_content.set<Component::Image> ("interface_action:image", world_width, m_action_height, 0, 0, 0);
  m_content.set<Component::Position> ("interface_action:position", Point(0, world_height));
  
  m_content.set<Component::Image> ("interface_verbs:image", interface_width, interface_height, 0, 0, 0);
  m_content.set<Component::Position> ("interface_verbs:position", Point(world_width, 0));
                                                                        
  m_content.set<Component::Image> ("interface_inventory:image", interface_width, interface_height, 0, 0, 0);
  m_content.set<Component::Position> ("interface_inventory:position",
                                      Point(world_width, interface_height));
  
  m_content.set<Component::Position>("chosen_verb:position", Point(world_width / 2,
                                                                   world_height + m_action_height / 2));

  m_content.get<Component::Image> ("turnicon:image")->on() = false;
  
  
  config().interface_width = interface_width;
  config().interface_height = m_action_height;
}

void Interface::interface_standard()
{
  int world_width = 1920;
  int world_height = 1000;
  m_action_height = 50;
  int interface_height = 150;
  int interface_width = world_width / 2;

  m_content.set<Component::Image> ("interface_action:image", world_width, m_action_height, 0, 0, 0);
  m_content.set<Component::Position> ("interface_action:position", Point(0, world_height));
  
  m_content.set<Component::Image> ("interface_verbs:image", interface_width, interface_height, 0, 0, 0);
  m_content.set<Component::Position> ("interface_verbs:position", Point(0, m_action_height + world_height));
                                                                        
  m_content.set<Component::Image> ("interface_inventory:image", interface_width, interface_height, 0, 0, 0);
  m_content.set<Component::Position> ("interface_inventory:position",
                                      Point(interface_width, m_action_height + world_height));
  
  m_content.set<Component::Position>("chosen_verb:position", Point(world_width / 2,
                                                                   world_height + m_action_height / 2));

  m_content.get<Component::Image> ("turnicon:image")->on() = false;

  config().interface_width = 0;
  config().interface_height = interface_height + m_action_height;
}

void Interface::interface_square()
{
  int world_width = 1920;
  int world_height = 1000;
  m_action_height = 50;
  int interface_height = 300;
  int interface_width = world_width;

  m_content.set<Component::Image> ("interface_action:image", world_width, m_action_height, 0, 0, 0);
  m_content.set<Component::Position> ("interface_action:position", Point(0, world_height));
  
  m_content.set<Component::Image> ("interface_verbs:image", interface_width, interface_height, 0, 0, 0);
  m_content.set<Component::Position> ("interface_verbs:position", Point(0, m_action_height + world_height));
                                                                        
  m_content.set<Component::Image> ("interface_inventory:image", interface_width, interface_height, 0, 0, 0);
  m_content.set<Component::Position> ("interface_inventory:position",
                                      Point(0, m_action_height + world_height + interface_height));
  
  m_content.set<Component::Position>("chosen_verb:position", Point(world_width / 2,
                                                                   world_height + m_action_height / 2));

  m_content.get<Component::Image> ("turnicon:image")->on() = false;

  config().interface_width = 0;
  config().interface_height = 2 * interface_height + m_action_height;
}

void Interface::interface_portrait()
{
  int world_width = 1920;
  int world_height = 1000;
  m_action_height = 100;
  int interface_height = 300;
  int interface_width = world_width;

  m_content.set<Component::Image> ("interface_action:image", world_width, m_action_height, 0, 0, 0);
  m_content.set<Component::Position> ("interface_action:position", Point(0, world_height));
  
  m_content.set<Component::Image> ("interface_verbs:image", interface_width, interface_height, 0, 0, 0);
  m_content.set<Component::Position> ("interface_verbs:position", Point(0, m_action_height + world_height));
                                                                        
  m_content.set<Component::Image> ("interface_inventory:image", interface_width, interface_height, 0, 0, 0);
  m_content.set<Component::Position> ("interface_inventory:position",
                                      Point(0, m_action_height + world_height + interface_height));
  
  m_content.set<Component::Position>("chosen_verb:position", Point(world_width / 2,
                                                                   world_height + m_action_height / 2));

  Component::Image_handle turnicon
    = m_content.get<Component::Image> ("turnicon:image");

  m_content.set<Component::Position>("turnicon:position",
                                     Point(0, world_height + 2 * interface_height + m_action_height));

  turnicon->on() = true;

  config().interface_width = 0;
  config().interface_height = 2 * interface_height + m_action_height + turnicon->height();
}

void Interface::vertical_layout()
{
  Component::Font_handle interface_font = m_content.get<Component::Font> ("interface:font");
  
  int top_width = 0;
  Component::Image_handle interface_verbs
    = m_content.get<Component::Image>("interface_verbs:image");
  int interface_width = interface_verbs->width();
  int interface_height = interface_verbs->height();

  int min_w_spacing = 30;
  int min_h_spacing = 15;
  
  int verbs_width = 0;
  int verbs_height = 0;

  for (std::size_t i = 0; i < m_verbs.size(); ++ i)
    m_verbs[i]->set_scale(1);

  for (std::size_t i = 0; i < m_verbs.size(); ++ i)
  {
    int w = m_verbs[i]->width();
    int h = m_verbs[i]->height() + min_h_spacing;
    verbs_width = (std::max)(verbs_width, w);
    verbs_height += h;
  }

  int min_verbs_width = verbs_width + min_w_spacing * 2;
  int min_verbs_height = verbs_height + min_h_spacing * 2;

  double w_scaling = interface_width / double(min_verbs_width);
  double h_scaling = interface_height / double(min_verbs_height);

  std::cerr << "Scaling = " << w_scaling << "*" << h_scaling << std::endl;

  double min_scaling = (std::min)(h_scaling, w_scaling);

  int h_spacing = interface_height;
  std::cerr << h_spacing << " ";
  for (std::size_t i = 0; i < m_verbs.size(); ++ i)
  {
    int h = int(m_verbs[i]->height() * min_scaling);
    h_spacing -= h;
    m_verbs[i]->set_scale(min_scaling);
  }

  h_spacing /= 8;
  std::cerr << h_spacing << " ";

  int x = config().world_width + interface_width / 2;
  int current_y= h_spacing / 2;

  for (std::size_t i = 0; i < m_verbs.size(); ++ i)
  {
    Component::Image_handle img = m_verbs[i];

    int y = current_y + 0.5 * img->height() * min_scaling;

    m_content.set<Component::Position>(img->entity() + ":position", Point(x, y));

    current_y += img->height() * min_scaling + h_spacing;
  }

  m_content.set<Component::Position>("chosen_verb:position", Point(config().world_width / 2,
                                                                   config().world_height + m_action_height / 2));

  m_verb_scale = min_scaling;
}

void Interface::horizontal_layout()
{
  Component::Font_handle interface_font = m_content.get<Component::Font> ("interface:font");
  
  int top_width = 0;
  Component::Image_handle interface_verbs
    = m_content.get<Component::Image>("interface_verbs:image");
  int interface_width = interface_verbs->width();
  int interface_height = interface_verbs->height();

  int min_w_spacing = 40;
  int min_h_spacing = 15;
  
  int top_verbs_width = 0;
  int bottom_verbs_width = 0;
  int top_verbs_height = 0;
  int bottom_verbs_height = 0;
  for (std::size_t i = 0; i < m_verbs.size(); ++ i)
    m_verbs[i]->set_scale(1);

  for (std::size_t i = 0; i < m_verbs.size(); ++ i)
  {
    int w = m_verbs[i]->width() + min_w_spacing;
    int h = m_verbs[i]->height();
    if (i % 2 == 0)
    {
      top_verbs_width += w;
      top_verbs_height = (std::max)(top_verbs_height, h);
    }
    else
    {
      bottom_verbs_width += w;
      bottom_verbs_height = (std::max)(bottom_verbs_height, h);
    }
  }

  int min_verbs_width = (std::max)(top_verbs_width, bottom_verbs_width);
  int min_verbs_height = top_verbs_height + bottom_verbs_height + min_h_spacing * 2;

  double w_scaling = interface_width / double(min_verbs_width);
  double h_scaling = interface_height / double(min_verbs_height);

  std::cerr << "Scaling = " << w_scaling << "*" << h_scaling << std::endl;

  double min_scaling = (std::min)(h_scaling, w_scaling);

  int w_spacing_top = interface_width;
  int w_spacing_bottom = interface_width;
  for (std::size_t i = 0; i < m_verbs.size(); ++ i)
  {
    int w = int(m_verbs[i]->width() * min_scaling);
    if (i % 2 == 0)
      w_spacing_top -= w;
    else
      w_spacing_bottom -= w;
    m_verbs[i]->set_scale(min_scaling);
  }

  int h_spacing = interface_height - int(min_scaling * (top_verbs_height + top_verbs_height));

  w_spacing_top /= 4;
  w_spacing_bottom /= 4;
  h_spacing /= 2;

  int current_xtop = w_spacing_top / 2;
  int current_xbottom = w_spacing_bottom / 2;
  int current_ytop = config().world_height + m_action_height + (h_spacing / 2);
  int current_ybottom = current_ytop + int(min_scaling * top_verbs_height) + h_spacing;

  for (std::size_t i = 0; i < m_verbs.size(); i += 2)
  {
    Component::Image_handle top = m_verbs[i];
    Component::Image_handle bottom = m_verbs[i+1];

    int xtop = current_xtop + 0.5 * top->width() * min_scaling;
    int xbottom = current_xbottom + 0.5 * bottom->width() * min_scaling;
    
    int ytop = current_ytop + 0.5 * top->height() * min_scaling;
    int ybottom = current_ybottom + 0.5 * bottom->height() * min_scaling;

    m_content.set<Component::Position>(top->entity() + ":position", Point(xtop, ytop));
    m_content.set<Component::Position>(bottom->entity() + ":position", Point(xbottom, ybottom));

    current_xtop += top->width() * min_scaling + w_spacing_top;
    current_xbottom += bottom->width() * min_scaling + w_spacing_bottom;
  }

  m_content.set<Component::Position>("chosen_verb:position", Point(config().world_width / 2,
                                                                   config().world_height + m_action_height / 2));

  m_verb_scale = min_scaling;
}

void Interface::update_pause_screen()
{
  Component::Font_handle interface_font = m_content.get<Component::Font> ("interface:font");

  Component::Image_handle pause_screen_img
    = Component::make_handle<Component::Image>
    ("pause_screen:image",
     config().world_width + config().interface_width,
     config().world_height + config().interface_height,
     0, 0, 0, 192);
  pause_screen_img->z() += 10;
      
  // Create pause screen
  Component::Boolean_handle paused
    = m_content.get<Component::Boolean>("game:paused");
  
  Component::Conditional_handle pause_screen
    = m_content.set<Component::Conditional>("pause_screen:conditional", paused,
                                            pause_screen_img, Component::Handle());

  Component::Image_handle pause_text_img
    = Component::make_handle<Component::Image>("pause_text:image", interface_font, "FFFFFF", "PAUSE");
  pause_text_img->z() += 10;
  pause_text_img->set_relative_origin(0.5, 0.5);

  Component::Conditional_handle pause_text
    = m_content.set<Component::Conditional>("pause_text:conditional", paused,
                                            pause_text_img, Component::Handle());
    
  m_content.set<Component::Position>("pause_text:position", Point(config().world_width / 2,
                                                                  config().world_height / 2));

}

void Interface::detect_collision (Component::Position_handle cursor)
{
  // Deactive previous collisions
  if (m_collision)
  {
    if (m_collision->entity().find("verb_") == 0)
      m_collision->set_scale(m_verb_scale);
  }

  m_collision = Component::Image_handle();
  
  for (const auto& e : m_content)
    if (Component::Image_handle img
        = Component::cast<Component::Image>(e))
    {
      if (!img->on() ||
          img->id().find("character") == 0 ||
          img->id().find("debug") == 0 ||
          img->id().find("chosen_verb") == 0 ||
          img->id().find("interface_") == 0)
        continue;
      
      Component::Position_handle p = m_content.get<Component::Position>(img->entity() + ":position");

      Point screen_position = p->value() - img->core().scaling * Vector(img->origin());
      int xmin = screen_position.x();
      int ymin = screen_position.y();
      int xmax = xmin + (img->core().scaling * (img->xmax() - img->xmin()));
      int ymax = ymin + (img->core().scaling * (img->ymax() - img->ymin()));

      if (cursor->value().x() < xmin ||
          cursor->value().x() > xmax ||
          cursor->value().y() < ymin ||
          cursor->value().y() > ymax)
        continue;

      if (img->id() != "background:image" && img->id().find("verb_") != 0)
      {
        int x_in_image = cursor->value().x() - xmin;
        int y_in_image = cursor->value().y() - ymin;
        if (!img->is_target_inside (x_in_image, y_in_image))
          continue;
      }
      
      // Now, collision happened
      
      if (m_collision)
      {
        // Keep image closest to screen
        if (img->z() > m_collision->z())
          m_collision = img;
      }
      else
        m_collision = img;

    }

}

void Interface::update_action ()
{
  std::string target_object = "";

  Component::Text_handle verb = m_content.get<Component::Text> ("chosen_verb:text");
  
  if (m_collision)
  {
    if (m_collision->entity().find("verb_") == 0)
      m_collision->set_scale(1.2 * m_verb_scale);
    if (Component::Text_handle name = m_content.request<Component::Text>(m_collision->entity() + ":name"))
    {
      target_object = name->value();
      Component::State_handle state = m_content.request<Component::State>(m_collision->entity() + ":state");
      if (state && state->value() == "inventory"
          && verb->entity() == "verb_goto")
      {
        verb = m_content.get<Component::Text>("verb_look:text");
      }
    }
  }

  if (!m_content.request<Component::Action>("character:action")
      || verb != m_content.get<Component::Text>("verb_goto:text"))
  {  
    Component::Image_handle text_img
      = m_content.set<Component::Image>("chosen_verb:image",
                                        m_content.get<Component::Font>("interface:font"), "FFFFFF",
                                        verb->value() + " " +
                                        target_object);
    text_img->set_relative_origin(0.5, 0.5);
    text_img->set_scale(0.8 * m_action_height / text_img->height());
  }
}

void Interface::update_inventory ()
{
  Component::Image_handle background = m_content.get<Component::Image>("interface_inventory:image");
  
  Component::Inventory_handle inventory = m_content.get<Component::Inventory>("game:inventory");

  Component::Position_handle inv_pos = m_content.get<Component::Position>("interface_inventory:position");

  std::size_t position = inventory->position();
  for (std::size_t i = 0; i < inventory->size(); ++ i)
  {
    Component::Image_handle img = m_content.get<Component::Image>(inventory->get(i) + ":conditional_image");
    
    if (position <= i && i < position + config().displayed_inventory_size)
    {
      std::size_t pos = i - position;
      double relative_pos = (1 + pos) / double(config().displayed_inventory_size + 1);
      
      img->on() = true;

      int x, y;

      if (m_layout == WIDESCREEN)
      {
        int width = img->width();
        int inv_width = background->width();
        int target_width = int(0.8 * inv_width);

        img->set_scale (target_width / double(width));

        x = inv_pos->value().x() + background->width() / 2;
        y = inv_pos->value().y() + int(relative_pos * background->height());
      }
      else
      {
        int height = img->height();
        int inv_height = background->height();
        int target_height = int(0.8 * inv_height);

        img->set_scale (target_height / double(height));

        x = inv_pos->value().x() + int(relative_pos * background->width());
        y = inv_pos->value().y() + background->height() / 2;
      }
      
      m_content.set<Component::Position>(inventory->get(i) + ":position", Point(x,y));


    }
    else
      img->on() = false;
  }
}


} // namespace Sosage::System

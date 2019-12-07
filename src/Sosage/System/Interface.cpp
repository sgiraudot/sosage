#include <Sosage/Component/Action.h>
#include <Sosage/Component/Position.h>
#include <Sosage/Component/Text.h>
#include <Sosage/System/Interface.h>

namespace Sosage::System
{

Interface::Interface (Content& content)
  : m_content (content)
  , m_action_min_height(50)
  , m_interface_min_height(150)
{

}

void Interface::main()
{
  Component::Boolean_handle rescaled
    = m_content.request<Component::Boolean>("window:rescaled");
  if (rescaled && rescaled->value())
    update_responsive();
  
  Component::Position_handle mouse
    = m_content.request<Component::Position>("mouse:position");
  if (mouse)
    detect_collision (mouse);

  Component::Position_handle clicked
    = m_content.request<Component::Position>("mouse:clicked");
  if (clicked && m_collision)
  {
    m_content.remove("mouse:clicked");
    
    if (m_collision->entity().find("verb_") == 0)
    {
      m_content.get<Component::Variable> ("chosen_verb:text")
        ->set(m_content.get<Component::Text>(m_collision->entity() + ":text"));
      m_content.remove("mouse:clicked");
    }
    else
    {
      std::string verb
        = m_content.get<Component::Text> ("chosen_verb:text")->entity();
      verb = std::string (verb.begin() + 5, verb.end());

      if (verb == "goto")
        m_content.set<Component::Variable>("mouse:target", m_collision);
      else
      {
        Component::Action_handle action
          = m_content.request<Component::Action> (m_collision->entity() + ":" + verb);
        if (action)
          m_content.set<Component::Variable>("character:action", action);

        m_content.remove("mouse:clicked");
      }
      
      m_content.get<Component::Variable> ("chosen_verb:text")
        ->set(m_content.get<Component::Text>("verb_goto:text"));
    }
  }

  update_action();

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
    m_verbs.push_back (verb_img);
    verb_img->set_relative_origin(0.5, 0.5);
  }

  Component::Text_handle
    verb_goto = m_content.set<Component::Text> ("verb_goto:text", "Aller vers");
  m_content.set<Component::Variable>("chosen_verb:text", verb_goto);

  // Create pause screen
  Component::Boolean_handle paused
    = m_content.set<Component::Boolean>("game:paused", false);

  m_content.set<Component::Position>("pause_screen:position", Point(0, 0));

  update_responsive();
}

void Interface::update_responsive()
{
  int world_width = config().world_width;
  int world_height = config().world_height;
  int window_width = world_width;
  int window_height = int(config().window_height * world_width / double(config().window_width));

  Component::Font_handle interface_font = m_content.get<Component::Font> ("interface:font");
  
  std::vector<Component::Image_handle> verbs;
  verbs.reserve(8);
  for (Component::Handle h : m_content)
    if (Component::Image_handle img = Component::cast<Component::Image>(h))
      verbs.push_back (img);

  int top_width = 0;

  int height = window_height - world_height;
  if (height < m_interface_min_height)
    height = m_interface_min_height;

  std::cerr << window_width << "*" << window_height << std::endl;
  int interface_height;
  int interface_width;
  Point inventory_origin;

  int interface_max_height = 350;

  if (window_height < window_width)
  {
    m_action_height = std::max (int(0.15 * height), m_action_min_height);
    interface_height = height - m_action_height;
    if (interface_height > interface_max_height)
      interface_height = interface_max_height;
    interface_width = window_width / 2;
    inventory_origin = Point (window_width / 2, world_height + m_action_height);
  }
  else
  {
    m_action_height = std::max (int(0.075 * height), m_action_min_height);
    interface_height = (height - m_action_height) / 2;
    if (interface_height > interface_max_height)
      interface_height = interface_max_height;
    interface_width = window_width;
    inventory_origin = Point (0, world_height + m_action_height + interface_height);
  }

  std::cerr << "Action height = " << m_action_height << std::endl
            << "Interface height = " << interface_height << std::endl;

  m_content.set<Component::Image> ("interface_action:image", world_width, m_action_height, 0, 0, 0);
  m_content.set<Component::Position> ("interface_action:position", Point(0, world_height));
  
  m_content.set<Component::Image> ("interface_verbs:image", interface_width, interface_height, 0, 0, 0);
  m_content.set<Component::Position> ("interface_verbs:position", Point(0, m_action_height + world_height));
                                                                        
  m_content.set<Component::Image> ("interface_inventory:image", interface_width, interface_height, 0, 0, 0);
  m_content.set<Component::Position> ("interface_inventory:position", inventory_origin);

  int min_w_spacing = 40;
  int min_h_spacing = 15;
  
  int top_verbs_width = 0;
  int bottom_verbs_width = 0;
  int top_verbs_height = 0;
  int bottom_verbs_height = 0;
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
  int current_ytop = world_height + m_action_height + (h_spacing / 2);
  int current_ybottom = current_ytop + int(min_scaling * top_verbs_height) + h_spacing;

  std::cerr << "Scaling = " << min_scaling << std::endl
            << "Spacing = " << w_spacing_top << " " << w_spacing_bottom << " " << h_spacing << std::endl;
  
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

  m_content.set<Component::Position>("chosen_verb:position", Point(world_width / 2,
                                                                   world_height + m_action_height / 2));
                                                                   
  Component::Image_handle pause_screen_img
    = Component::make_handle<Component::Image>
    ("pause_screen:image", window_width, window_height,
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
    
  m_content.set<Component::Position>("pause_text:position", Point(world_width / 2,
                                                                  world_height / 2));

  m_verb_scale = min_scaling;
  config().interface_height = height;
}

void Interface::detect_collision (Component::Position_handle mouse)
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

      if (mouse->value().x() < xmin ||
          mouse->value().x() > xmax ||
          mouse->value().y() < ymin ||
          mouse->value().y() > ymax)
        continue;

      if (img->id() != "background:image" && img->id().find("verb_") != 0)
      {
        int x_in_image = mouse->value().x() - xmin;
        int y_in_image = mouse->value().y() - ymin;
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
  
  if (m_collision)
  {
    if (m_collision->entity().find("verb_") == 0)
      m_collision->set_scale(1.2 * m_verb_scale);
    if (Component::Text_handle name = m_content.request<Component::Text>(m_collision->entity() + ":name"))
      target_object = name->value();
  }

  Component::Image_handle text_img
    = m_content.set<Component::Image>("chosen_verb:image",
                                      m_content.get<Component::Font>("interface:font"), "FFFFFF",
                                      m_content.get<Component::Text> ("chosen_verb:text")->value() + " " +
                                      target_object);
  text_img->set_relative_origin(0.5, 0.5);
  text_img->set_scale(0.8 * m_action_height / text_img->height());
}


} // namespace Sosage::System

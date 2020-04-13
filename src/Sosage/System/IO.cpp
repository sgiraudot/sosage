#include <Sosage/Component/Action.h>
#include <Sosage/Component/Animation.h>
#include <Sosage/Component/Code.h>
#include <Sosage/Component/Font.h>
#include <Sosage/Component/Ground_map.h>
#include <Sosage/Component/Image.h>
#include <Sosage/Component/Inventory.h>
#include <Sosage/Component/Music.h>
#include <Sosage/Component/Position.h>
#include <Sosage/Component/Sound.h>
#include <Sosage/Component/Text.h>
#include <Sosage/System/IO.h>
#include <Sosage/platform.h>
#include <Sosage/version.h>
#include <Sosage/Utils/color.h>
#include <Sosage/Utils/profiling.h>

namespace Sosage::System
{

IO::IO (Content& content)
  : m_content (content)
{
}

std::string IO::read_init (const std::string& folder_name)
{
  m_folder_name = folder_name;
  std::string file_name = folder_name + "resources" + Sosage::folder_separator + "init.yaml";

  Core::IO input (file_name);

  std::string v = input["version"].string();
  check (version(v) == SOSAGE_VERSION,
         "Error: room version " + v + " incompatible with Sosage " + Sosage::version());

#ifndef SOSAGE_ANDROID
  std::string cursor = input["cursor"].string("sprites", "png");
  Component::Image_handle cursor_img
    = m_content.set<Component::Image> ("cursor:image", local_file_name(cursor),
                                       Sosage::cursor_depth);
  cursor_img->set_relative_origin(0.5, 0.5);
#endif
  
  m_content.set<Component::Position> ("cursor:position", Point(0,0));
  
  std::string turnicon = input["turnicon"].string("sprites", "png");
  Component::Image_handle turnicon_img
    = m_content.set<Component::Image>("turnicon:image", local_file_name(turnicon), 0);
  turnicon_img->on() = false;

  std::string click_sound = input["click_sound"].string("sounds", "wav");
  m_content.set<Component::Sound>("click:sound", local_file_name(click_sound));
  
  std::string debug_font = input["debug_font"].string("fonts", "ttf");
  m_content.set<Component::Font> ("debug:font", local_file_name(debug_font), 40);

  std::string interface_font = input["interface_font"].string("fonts", "ttf");
  m_content.set<Component::Font> ("interface:font", local_file_name(interface_font), 80);
  
  std::string interface_color = input["interface_color"].string();
  m_content.set<Component::Text> ("interface:color", interface_color);

  std::array<unsigned char, 3> color = color_from_string (interface_color);

  for (std::size_t i = 0; i < input["inventory_arrows"].size(); ++ i)
  {
    std::string id = input["inventory_arrows" ][i].string("sprites", "png");
    Component::Image_handle arrow
      = m_content.set<Component::Image> ("inventory_arrow_" + std::to_string(i) + ":image",
                                         local_file_name(id),
                                         Sosage::inventory_front_depth);
    arrow->set_relative_origin(0.5, 0.5);
    Component::Image_handle arrow_background
      = m_content.set<Component::Image> ("inventory_arrow_background_" + std::to_string(i) + ":image",
                                         arrow->width(), arrow->height(),
                                         color[0], color[1], color[2]);
    arrow_background->set_relative_origin(0.5, 0.5);
    arrow_background->z() = Sosage::inventory_back_depth;
  }

  for (std::size_t i = 0; i < input["default"].size(); ++ i)
  {
    const Core::IO::Node& idefault = input["default"][i];
    std::string id = idefault["action"].string();

    Component::Random_conditional_handle action
      = m_content.set<Component::Random_conditional>("default:" + id);

    for (std::size_t j = 0; j < idefault["effect"].size(); ++ j)
    {
      const Core::IO::Node& iaction = idefault["effect"][j];
      Component::Action_handle rnd_action = Component::make_handle<Component::Action>
        ("default:" + id + "_" + std::to_string(j));
      rnd_action->add ({ "look" });
      rnd_action->add (iaction.string_array());

      action->add (1.0, rnd_action);
    }

  }

    
  return input["load_room"].string("resources", "yaml");
}

void IO::read_character (const std::string& file_name, int x, int y)
{
  Core::IO input (local_file_name(file_name));

  std::string name = input["name"].string();

  std::string mouth = input["mouth"]["skin"].string("sprites", "png");
  int mdx_right = input["mouth"]["dx_right"].integer();
  int mdx_left = input["mouth"]["dx_left"].integer();
  int mdy = input["mouth"]["dy"].integer();
  
  std::string head = input["head"]["skin"].string("sprites", "png");
  int hdx_right = input["head"]["dx_right"].integer();
  int hdx_left = input["head"]["dx_left"].integer();
  int hdy = input["head"]["dy"].integer();
  
  std::string body = input["body"]["skin"].string("sprites", "png");
  
  Component::Animation_handle abody
    = m_content.set<Component::Animation>("character_body:image", local_file_name(body),
                                          0, 8, 6);
  abody->set_relative_origin(0.5, 0.95);
  
  Component::Animation_handle ahead
    = m_content.set<Component::Animation>("character_head:image", local_file_name(head),
                                          0, 7, 2);
  ahead->set_relative_origin(0.5, 1.0);
  
  Component::Animation_handle amouth
    = m_content.set<Component::Animation>("character_mouth:image", local_file_name(mouth),
                                          0, 11, 2);
  amouth->set_relative_origin(0.5, 1.0);
  
  Component::Position_handle pbody
    = m_content.set<Component::Position>("character_body:position", Point(x, y));

  m_content.set<Component::Position>("character_head:gap_right", Point(hdx_right,hdy));
  m_content.set<Component::Position>("character_head:gap_left", Point(hdx_left,hdy));

  m_content.set<Component::Position>("character_mouth:gap_right", Point(mdx_right,mdy));
  m_content.set<Component::Position>("character_mouth:gap_left", Point(mdx_left,mdy));
  
  Component::Position_handle phead
    = m_content.set<Component::Position>("character_head:position", Point(x - hdx_right, y - hdy));

  Component::Position_handle pmouth
    = m_content.set<Component::Position>("character_mouth:position", Point(x - hdx_right - mdx_right,
                                                                           y - hdy - mdy));

  Component::Ground_map_handle ground_map
    = m_content.get<Component::Ground_map>("background:ground_map");
  
  Point pos_body = pbody->value();

  double z_at_point = ground_map->z_at_point (pos_body);
  abody->rescale (z_at_point);
  
  ahead->rescale (z_at_point);
  ahead->z() += 1;
  phead->set (pbody->value() - abody->core().scaling * Vector(hdx_right, hdy));
  
  amouth->rescale (z_at_point);
  amouth->z() += 2;
  pmouth->set (phead->value() - ahead->core().scaling * Vector(mdx_right, mdy));

}

std::string IO::read_room (const std::string& file_name)
{
  Timer t ("Room reading");
  t.start();
  Core::IO input (local_file_name(file_name));

  std::string name = input["name"].string();
  std::string music = input["music"].string("musics", "ogg");
  m_content.set<Component::Music>("game:music", local_file_name(music));
  
  std::string background = input["background"].string("backgrounds", "png");
  std::string ground_map = input["ground_map"].string("backgrounds", "png");
  int front_z = input["front_z"].integer();
  int back_z = input["back_z"].integer();

  Component::Image_handle background_img
    = m_content.set<Component::Image>("background:image", local_file_name(background), 0);
  background_img->box_collision() = true;
  
  m_content.set<Component::Position>("background:position", Point(0, 0));
  m_content.set<Component::Ground_map>("background:ground_map", local_file_name(ground_map),
                                       front_z, back_z);

  std::string character = input["character"].string("resources", "yaml");
  int x = input["coordinates"][0].integer();
  int y = input["coordinates"][1].integer();
  read_character (character, x, y);

  if (input.has("booleans"))
    for (std::size_t i = 0; i < input["booleans"].size(); ++ i)
    {

    }

  if (input.has("scenery"))
    for (std::size_t i = 0; i < input["scenery"].size(); ++ i)
    {
      const Core::IO::Node& iscenery = input["scenery"][i];
      std::string id = iscenery["id"].string();
      int x = iscenery["coordinates"][0].integer();
      int y = iscenery["coordinates"][1].integer();
      int z = iscenery["coordinates"][2].integer();
      std::string skin = iscenery["skin"].string("sprites", "png");
      
      Component::Position_handle pos
        = m_content.set<Component::Position>(id + ":position", Point(x,y));
      Component::Image_handle img
        = m_content.set<Component::Image>(id + ":image", local_file_name(skin), 0);
      img->set_relative_origin(0.5, 1.0);
      img->z() = z;
      debug("Scenery " + id + " at position " + std::to_string(img->z()));

    }
  
  if (input.has("objects"))
    for (std::size_t i = 0; i < input["objects"].size(); ++ i)
    {
      const Core::IO::Node& iobject = input["objects"][i];
      
      std::string id = iobject["id"].string();
      std::string name = iobject["name"].string();
      int x = iobject["coordinates"][0].integer();
      int y = iobject["coordinates"][1].integer();
      int z = iobject["coordinates"][2].integer();
      int vx = iobject["view"][0].integer();
      int vy = iobject["view"][1].integer();
      bool box_collision = iobject["box_collision"].boolean();
      
      m_content.set<Component::Text>(id + ":name", name);
      Component::State_handle state_handle
        = m_content.set<Component::State>(id + ":state");
      Component::Position_handle pos
        = m_content.set<Component::Position>(id + ":position", Point(x,y));
      m_content.set<Component::Position>(id + ":view", Point(vx,vy));

      Component::State_conditional_handle conditional_handle;

      for (std::size_t j = 0; j < iobject["states"].size(); ++ j)
      {
        const Core::IO::Node& istate = iobject["states"][j];

        std::string state = istate["id"].string();

        // init with first state found
        bool init = false;
        if (state_handle->value() == "")
        {
          state_handle->set(state);
          conditional_handle = m_content.set<Component::State_conditional>(id + ":image", state_handle);
          init = true;
        }
        else
          conditional_handle = m_content.get<Component::State_conditional>(id + ":image");

        if (state == "none")
          conditional_handle->add(state, nullptr);
        else
        {
          std::string skin = istate["skin"].string("sprites", "png");
          Component::Image_handle img
            = Component::make_handle<Component::Image>(id + ":conditional_image", local_file_name(skin), 0);
          img->set_relative_origin(0.5, 1.0);
          img->box_collision() = box_collision;

          img->z() = z;
          debug("Object " + id + ":" + state + " at position " + std::to_string(img->z()));

          conditional_handle->add(state, img);
        }
      }
      
      if (state_handle->value() == "inventory")
      {
        m_content.get<Component::Inventory>("game:inventory")->add(id);
        Component::Image_handle img
          = m_content.get<Component::Image>(id + ":image");
        img->set_relative_origin(0.5, 0.5);
        img->z() = Sosage::inventory_back_depth;
      }
    }
  
  if (input.has("actions"))
    for (std::size_t i = 0; i < input["actions"].size(); ++ i)
    {
      const Core::IO::Node& iaction = input["actions"][i];

      std::string id = iaction["id"].string();
      std::string target = iaction["target"].string();

      if (iaction.has("source"))
      {
        id = id + "_" + iaction["source"].string();
        std::cerr << target << ":" << id << std::endl;
      }

      Component::Action_handle action;
              
      if (iaction.has("state"))
      {
        std::string state = iaction["state"].string();
        Component::State_conditional_handle conditional_handle
          = m_content.request<Component::State_conditional>(target + ":" + id);

        if (!conditional_handle)
        {
          Component::State_handle state_handle
            = m_content.get<Component::State>(target + ":state");
          conditional_handle
            = m_content.set<Component::State_conditional>(target + ":" + id, state_handle);
        }

        action = Component::make_handle<Component::Action>(target + ":" + id + ":" + state);
        conditional_handle->add (state, action);
      }
      else
        action = m_content.set<Component::Action>(target + ":" + id);


      for (std::size_t j = 0; j < iaction["effect"].size(); ++ j)
        action->add (iaction["effect"][j].string_array());
    }

  if (input.has("windows"))
    for (std::size_t i = 0; i < input["windows"].size(); ++ i)
    {
      const Core::IO::Node& iwindow = input["windows"][i];

      std::string id = iwindow["id"].string();
      std::string skin = iwindow["skin"].string("sprites", "png");
      
      Component::Image_handle img
        = m_content.set<Component::Image>(id + ":image", local_file_name(skin), 0);
      img->set_relative_origin(0.5, 0.5);
      img->z() = Sosage::inventory_front_depth;
      img->on() = false;
      
      m_content.set<Component::Position>(id + ":position",
                                         Point(Sosage::world_width / 2,
                                               Sosage::world_height / 2));
    }
  
  if (input.has("codes"))
    for (std::size_t i = 0; i < input["codes"].size(); ++ i)
    {
      const Core::IO::Node& icode = input["codes"][i];

      std::string id = icode["id"].string();
      std::string button_sound = icode["button_sound"].string("sounds", "wav");
      std::string success_sound = icode["success_sound"].string("sounds", "wav");
      std::string failure_sound = icode["failure_sound"].string("sounds", "wav");

      Component::Code_handle code
        = m_content.set<Component::Code>(id + ":code");
      
      Component::State_handle state_handle
        = m_content.set<Component::State>(id + ":state");
      Component::State_conditional_handle conditional_handle_off;
      Component::State_conditional_handle conditional_handle_on;

      for (std::size_t j = 0; j < icode["states"].size(); ++ j)
      {
        const Core::IO::Node& istate = icode["states"][j];

        std::string state = istate["id"].string();

        // init with first state found
        bool init = false;
        if (state_handle->value() == "")
        {
          state_handle->set(state);
          conditional_handle_off
            = m_content.set<Component::State_conditional>(id + ":image", state_handle);
          conditional_handle_on
            = m_content.set<Component::State_conditional>(id + "_button:image", state_handle);
          init = true;
        }
        else
        {
          conditional_handle_off = m_content.get<Component::State_conditional>(id + ":image");
          conditional_handle_on = m_content.get<Component::State_conditional>(id + "_button:image");
        }

        std::string skin_off = istate["skin"][0].string("sprites", "png");
        Component::Image_handle img_off
          = Component::make_handle<Component::Image>(id + ":conditional_image", local_file_name(skin_off), 0);
        img_off->set_relative_origin(0.5, 0.5);
        img_off->z() = Sosage::inventory_back_depth;
        img_off->on() = false;
        
        m_content.set<Component::Position>(id + ":position",
                                           Point(Sosage::world_width / 2,
                                                 Sosage::world_height / 2));

        std::string skin_on = istate["skin"][0].string("sprites", "png");
        Component::Image_handle img_on
          = Component::make_handle<Component::Image>(id + "_button:conditional_image", local_file_name(skin_off), 0);
        img_on->set_relative_origin(0.5, 0.5);
        img_on->z() = Sosage::inventory_front_depth;
        img_on->on() = false;

        m_content.set<Component::Position>(id + "_button:position",
                                           Point(Sosage::world_width / 2,
                                                 Sosage::world_height / 2));

        conditional_handle_off->add(state, img_off);
        conditional_handle_on->add(state, img_on);
      }
      
      for (std::size_t j = 0; j < icode["buttons"].size(); ++ j)
      {
        const Core::IO::Node& ibutton = icode["buttons"][j];

        std::string value = ibutton["value"].string();

        const Core::IO::Node& coordinates = ibutton["coordinates"];
        code->add_button (value,
                          coordinates[0].integer(),
                          coordinates[1].integer(),
                          coordinates[2].integer(),
                          coordinates[3].integer());
      }
      
      for (std::size_t j = 0; j < icode["answer"].size(); ++ j)
        code->add_answer_item (icode["answer"][j].string());
      
      Component::Action_handle action
        = m_content.set<Component::Action> (id + ":action");
      for (std::size_t j = 0; j < icode["on_success"].size(); ++ j)
        action->add (icode["on_success"][j].string_array());
    }
  
  t.stop();
  return std::string();
}

std::string IO::local_file_name (const std::string& file_name) const
{
  return m_folder_name + file_name;
}



} // namespace Sosage::System

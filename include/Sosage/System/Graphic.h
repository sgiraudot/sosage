#ifndef SOSAGE_SYSTEM_GRAPHIC_H
#define SOSAGE_SYSTEM_GRAPHIC_H

#include <Sosage/Component/Image.h>
#include <Sosage/Core/Graphic.h>
#include <Sosage/Content.h>

#include <vector>

namespace Sosage::System
{

class Graphic
{
private:

  Content& m_content;
  Core::Graphic m_core;

public:

  Graphic (Content& content, const std::string& name);

  void main();

private:

  void get_images (std::vector<Component::Image_handle>& images);

  void display_images (std::vector<Component::Image_handle>& images);

  void display_path();
};

} // namespace Sosage::System

#endif // SOSAGE_SYSTEM_GRAPHIC_H

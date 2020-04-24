#ifndef SOSAGE_COMPONENT_CROPPED_H
#define SOSAGE_COMPONENT_CROPPED_H

#include <Sosage/Component/Handle.h>
#include <Sosage/Component/Image.h>

#include <vector>

namespace Sosage::Component
{

class Cropped : public Image
{
public:

  int m_xmin;
  int m_xmax;
  int m_ymin;
  int m_ymax;

public:

  Cropped (const std::string& id, const std::string& file_name, int z);

  void crop (int xmin, int xmax, int ymin, int ymax);
  
  virtual int xmin() const { return m_xmin; }
  virtual int xmax() const { return m_xmax; }
  virtual int ymin() const { return m_ymin; }
  virtual int ymax() const { return m_ymax; }
};

typedef std::shared_ptr<Cropped> Cropped_handle;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_CROPPED_H

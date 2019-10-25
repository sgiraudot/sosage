#ifndef SOSAGE_COMPONENT_ANIMATION_H
#define SOSAGE_COMPONENT_ANIMATION_H

#include <Sosage/Component/Handle.h>
#include <Sosage/Component/Image.h>

#include <vector>

namespace Sosage::Component
{

class Animation : public Image
{
public:

  struct Frame
  {
    int x;
    int y;
    int duration;
    int ellapsed;
    Frame (int x = 0, int y = 0, int duration = 1)
      : x(x), y(y), duration(duration), ellapsed(0)
    { }
  };

private:
  
  const int m_width_subdiv;
  const int m_height_subdiv;
  std::vector<Frame> m_frames;
  std::size_t m_current;

public:

  Animation (const std::string& id, const std::string& file_name, int z,
             int width_subdiv, int height_subdiv);

  void reset();
  
  virtual int xmin() const;
  virtual int xmax() const;
  virtual int ymin() const;
  virtual int ymax() const;

  const std::vector<Frame>& frames() const { return m_frames; }
  std::vector<Frame>& frames() { return m_frames; }

  void next_frame()
  {
    if (++ m_frames[m_current].ellapsed == m_frames[m_current].duration)
    {
      m_frames[m_current].ellapsed = 0;
      if (++ m_current == m_frames.size())
        m_current = 0;
    }
  }
};

typedef std::shared_ptr<Animation> Animation_handle;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_ANIMATION_H

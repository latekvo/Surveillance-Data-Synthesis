#pragma once

namespace AS {

class BaseComponent {
 public:
  virtual void draw() = 0;
  virtual void postRender() {};
};

}  // namespace AS

#pragma once

class IUIManager {
public:
  virtual ~IUIManager() = default;
  virtual void setMenu(bool active) = 0;
  virtual bool isMenuActive() const = 0;
};

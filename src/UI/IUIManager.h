#pragma once

class IUIManager {
public:
  virtual ~IUIManager() = default;
  virtual void trySetMenu(bool active) = 0;
  virtual bool isMenuActive() const = 0;
};

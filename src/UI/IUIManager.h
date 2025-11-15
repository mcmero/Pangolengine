#pragma once

class IUIManager {
public:
  virtual ~IUIManager() = default;
  virtual void trySetMenu(bool active) = 0;
  virtual bool isMenuActive() const = 0;
  virtual void setRequestExit(bool v) = 0;
  virtual bool getRequestExit() const = 0;
};

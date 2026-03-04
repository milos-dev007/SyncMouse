#pragma once

#include <QObject>

#include "core/InputEvent.h"

namespace syncmouse {

class InputInjector : public QObject {
  Q_OBJECT
public:
  explicit InputInjector(QObject* parent = nullptr) : QObject(parent) {}
  ~InputInjector() override = default;

  virtual void inject(const InputEvent& event) = 0;
  void setEnabled(bool enabled) { enabled_ = enabled; }
  bool isEnabled() const { return enabled_; }

protected:
  bool enabled_ = true;
};

} // namespace syncmouse

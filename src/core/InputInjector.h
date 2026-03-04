#pragma once

#include <QObject>
#include <QtGlobal>

#include <algorithm>

#include "core/InputEvent.h"

namespace syncmouse {

class InputInjector : public QObject {
  Q_OBJECT
public:
  enum class ReturnEdge : quint8 { None = 0, Left, Right, Up, Down };

  explicit InputInjector(QObject* parent = nullptr) : QObject(parent) {}
  ~InputInjector() override = default;

  virtual void inject(const InputEvent& event) = 0;
  void setEnabled(bool enabled) { enabled_ = enabled; }
  bool isEnabled() const { return enabled_; }
  void setMouseScale(double scale) { mouseScale_ = scale; }
  double mouseScale() const { return mouseScale_; }
  void setReturnEdge(ReturnEdge edge) {
    returnEdge_ = edge;
    returnArmed_ = false;
  }
  ReturnEdge returnEdge() const { return returnEdge_; }
  void setReturnEdgeMargin(int px) { returnEdgeMargin_ = std::max(0, px); }

signals:
  void returnRequested();

protected:
  void maybeRequestReturn(int x, int y, int left, int top, int right, int bottom, bool yIncreasesDown) {
    if (returnEdge_ == ReturnEdge::None) {
      return;
    }

    const int minX = std::min(left, right);
    const int maxX = std::max(left, right);
    const int minY = std::min(top, bottom);
    const int maxY = std::max(top, bottom);
    const int margin = returnEdgeMargin_;

    bool inBand = false;
    switch (returnEdge_) {
      case ReturnEdge::Left:
        inBand = x <= minX + margin;
        break;
      case ReturnEdge::Right:
        inBand = x >= maxX - margin;
        break;
      case ReturnEdge::Up:
        inBand = yIncreasesDown ? (y <= minY + margin) : (y >= maxY - margin);
        break;
      case ReturnEdge::Down:
        inBand = yIncreasesDown ? (y >= maxY - margin) : (y <= minY + margin);
        break;
      default:
        break;
    }

    if (!returnArmed_) {
      if (!inBand) {
        returnArmed_ = true;
      }
      return;
    }

    if (inBand) {
      returnArmed_ = false;
      emit returnRequested();
    }
  }

  bool enabled_ = true;
  double mouseScale_ = 1.0;
  ReturnEdge returnEdge_ = ReturnEdge::None;
  bool returnArmed_ = false;
  int returnEdgeMargin_ = 2;
};

} // namespace syncmouse

#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// Система координат: read-only, ОБЩАЯ для множества траекторий
// (создаётся один раз на «мире» симуляции).
struct ReferenceFrame {
  std::string name;
};

// Точка: (x, y) в момент t.
struct Point {
  double x;
  double y;
  double t;
};

// Trajectory (мини-версия): последовательность точек в одной frame.
//
// Review: копирование = delete (тяжёлый), но move-операций НЕТ.
// Как только копирование объявлено пользователем (даже = delete),
// компилятор move НЕ генерирует -> vector<SimpleTrajectory> не
// компилируется. ТЗ 1: дописать move-конструктор и move-присваивание,
// noexcept; исходник после move — валиден, Empty() == true.
class SimpleTrajectory {
 public:
  SimpleTrajectory() = default;
  explicit SimpleTrajectory(std::shared_ptr<const ReferenceFrame> frame)
      : frame_(std::move(frame)) {}

  SimpleTrajectory(const SimpleTrajectory&) = delete;
  SimpleTrajectory& operator=(const SimpleTrajectory&) = delete;

  // TODO (ТЗ 1): move-конструктор и move-присваивание, noexcept.

  void AddPoint(double x, double y, double t) {
    points_.push_back(Point{x, y, t});
  }

  bool Empty() const { return points_.empty(); }
  std::size_t Size() const { return points_.size(); }

  const ReferenceFrame* Frame() const { return frame_.get(); }
  const std::shared_ptr<const ReferenceFrame>& FramePtr() const {
    return frame_;
  }

 private:
  std::shared_ptr<const ReferenceFrame> frame_;
  std::vector<Point> points_;
};

// Партия траекторий: переезжает между модулями (IPC).
//
// ТЗ 2:
//   - Add(SimpleTrajectory) — by value + move (забирает владение);
//   - ExtractAll() — consumer забирает всё, батч пустеет;
//   - batch move-only: копии = delete, move-операции noexcept.
class TrajectoryBatch {
 public:
  explicit TrajectoryBatch(std::shared_ptr<const ReferenceFrame> frame)
      : frame_(std::move(frame)) {}

  // Копия запрещена (тяжёлый trajs_). TODO (ТЗ 2): добавить
  // move-конструктор/присваивание (noexcept) — без них
  // vector<TrajectoryBatch> и перенос батча не компилируются.
  TrajectoryBatch(const TrajectoryBatch&) = delete;
  TrajectoryBatch& operator=(const TrajectoryBatch&) = delete;

  void Add(SimpleTrajectory traj) {  // TODO: std::move(traj)
    trajs_.push_back(std::move(traj));
  }

  std::size_t Size() const { return trajs_.size(); }

  // TODO (ТЗ 2): вернуть std::move(trajs_) (батч пустеет).
  std::vector<SimpleTrajectory> ExtractAll() { return trajs_; }

  const std::shared_ptr<const ReferenceFrame>& Frame() const {
    return frame_;
  }

 private:
  std::shared_ptr<const ReferenceFrame> frame_;
  std::vector<SimpleTrajectory> trajs_;
};

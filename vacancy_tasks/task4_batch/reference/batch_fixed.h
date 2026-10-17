#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// Система координат: read-only, ОБЩАЯ для множества траекторий.
// shared_ptr<const ...>: владение реально разделено (мир + N траекторий),
// const — содержимое не мутирует после создания.
struct ReferenceFrame {
  std::string name;
};

// Точка: (x, y) в момент t.
struct Point {
  double x;
  double y;
  double t;
};

// Trajectory вация: move-only.
//
// Почему move не генерируется сам: user-declared copy operations
// (даже = delete) SUPPRESS move-генерацию (C++11 [class.copy.ctor]/4,
// в C++11; в C++20 правило смягчено, но поведение то же для
// user-declared).
//
// Что делает move:
//   frame_ (shared_ptr): атомарно передаёт указатель + счётчик
//   (control block общий), исходник обнуляется;
//   points_ (vector): переезжают 3 указателя (begin/end/capacity) —
//   O(1), без выделений и без memcpy данных;
//   (если был бы кэш unique_ptr: указатель переезжает, исходник null.)
//
// Почему noexcept: vector<SimpleTrajectory>::push_back при реаллокации
// переносит элементы: если move НЕ noexcept, стандарт требует копию —
// а копия delete'd -> ошибка компиляции (или копия, если бы была).
class SimpleTrajectory {
 public:
  SimpleTrajectory() = default;
  explicit SimpleTrajectory(std::shared_ptr<const ReferenceFrame> frame)
      : frame_(std::move(frame)) {}

  SimpleTrajectory(const SimpleTrajectory&) = delete;
  SimpleTrajectory& operator=(const SimpleTrajectory&) = delete;

  // Исходник после move: frame_ = nullptr, points_.empty()
  // -> валидное состояние, Empty() == true.
  SimpleTrajectory(SimpleTrajectory&& other) noexcept
      : frame_(std::move(other.frame_)),
        points_(std::move(other.points_)) {}

  SimpleTrajectory& operator=(SimpleTrajectory&& other) noexcept {
    if (this != &other) {  // self-move-assignment guard
      frame_ = std::move(other.frame_);
      points_ = std::move(other.points_);
    }
    return *this;
  }

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
// Add(SimpleTrajectory) — by value + move.
//   batch.Add(std::move(t)): 2 move-конструкции (t -> параметр;
//   параметр -> вектор). Ни одной копии.
//   batch.Add(t) (lvalue): копия — но SimpleTrajectory move-only,
//   так что lvalue-вызов не скомпилируется: и хорошо, владение
//   уходить не должно.
//   Почему не T&&: теряется одна форма вызова для обоих видов
//   аргументов (нужны перегрузки); почему не const&:
//   тогда приходилось бы копировать, а копировать нельзя.
// ExtractAll — возвращает std::move(trajs_): consumer забирает данные,
// батч пустеет; return const& невозможен — caller (другой модуль/поток)
// может прожить дольше батча (IPC: батч — временный объект канала).
class TrajectoryBatch {
 public:
  explicit TrajectoryBatch(std::shared_ptr<const ReferenceFrame> frame)
      : frame_(std::move(frame)) {}

  TrajectoryBatch(const TrajectoryBatch&) = delete;
  TrajectoryBatch& operator=(const TrajectoryBatch&) = delete;
  TrajectoryBatch(TrajectoryBatch&&) noexcept = default;
  TrajectoryBatch& operator=(TrajectoryBatch&&) noexcept = default;

  void Add(SimpleTrajectory traj) {
    trajs_.push_back(std::move(traj));
  }

  std::size_t Size() const { return trajs_.size(); }

  // Bатч пустеет после вызова (контракт в комментарии — часть API).
  std::vector<SimpleTrajectory> ExtractAll() { return std::move(trajs_); }

  const std::shared_ptr<const ReferenceFrame>& Frame() const {
    return frame_;
  }

 private:
  std::shared_ptr<const ReferenceFrame> frame_;
  std::vector<SimpleTrajectory> trajs_;
};

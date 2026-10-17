#pragma once

#include <cmath>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

// Кинематическая абстракция: система координат. Только чтение, может
// быть общей (shared) для множества траекторий.
struct ReferenceFrame {
  std::string name;
};

// Точка траектории: позиция (x, y) в момент времени t.
// Последовательность точек отсортирована по t (t неубывающий).
struct Point {
  double x;
  double y;
  double t;
};

// Траектория: последовательность точек в одной системе координат,
// отсортированная по времени.
//
// Legacy. Код из 2021, пришёл «просто работает». Замечания ревьюера —
// в комментариях ниже, ТЗ — в main.cpp.
//
// Владельство (review #5, #6):
//   frame_  — shared_ptr: система координат общая, read-only, у неё
//             МНОЖЕСТВО владельцев; unique_ptr тут не подходит.
//   points_ — vector: эксклюзивное владение, умный указатель не нужен.
//   У класса НЕТ move-операций (review #3) — vector<Trajectory> не
//             скомпилируется, пока не сделан пункт 2.1 ТЗ.

class Trajectory {
 public:
  Trajectory() = default;
  explicit Trajectory(std::shared_ptr<const ReferenceFrame> frame)
      : frame_(std::move(frame)) {}

  // Класс намеренно сделали «некопируемым» (points_ — тяжёлый),
  // но move-операции не дописали.
  // Review #3: как только копирование объявлено пользователем
  // (даже через = delete), компилятор НЕ генерирует move-конструктор.
  // Объясните почему — устный вопрос ТЗ 2.1.
  Trajectory(const Trajectory&) = delete;
  Trajectory& operator=(const Trajectory&) = delete;

  // TODO (ТЗ 2.1): move-конструктор и move-присваивание, noexcept.
  // Исходник после move — в валидном состоянии: Empty() == true.

  // Мутация. НЕвалидирует кэш кумулятивных длин (ТЗ 2.2) —
  // legacy-кэша ещё нет, но кэш появится, и AddPoint обязан его сбросить.
  // Review #4 (контракт): не гарантируется неубывание t — в проде
  // прилетали перемешанные точки, и характеристики считались мусором.
  // ТЗ 2.5: решите, что делать с контрактом, и запишите решение.
  void AddPoint(double x, double y, double t) {
    points_.push_back(Point{x, y, t});
  }

  bool Empty() const { return points_.empty(); }
  std::size_t Size() const { return points_.size(); }

  const std::vector<Point>& Points() const { return points_; }
  const ReferenceFrame* Frame() const { return frame_.get(); }

  // Удаляет ВСЕ точки с t <= t_cut, остаток сохраняет в исходном
  // порядке.
  // Review #2 (производительность): erase на позиции i — O(n) (сдвиг
  // хвоста). Если удаляется много точек — O(n^2) на вызов. ТЗ: O(n).
  void TrimBefore(double t_cut) {
    for (std::size_t i = 0; i < points_.size(); ++i) {
      if (points_[i].t <= t_cut) {
        points_.erase(points_.begin() + i);
        --i;  // не проскакиваем следующий элемент
      }
    }
  }

  // result[i] — длина пути от points_[0] до points_[i]. result[0] == 0.
  // Review #7 (производительность): для КАЖДОГО i префикс пересчитывается
  // с нуля — O(n^2). ТЗ: O(n) + ленивый кэш (ТЗ 2.2).
  std::vector<double> CumulativeLength() const {
    std::vector<double> result;
    result.reserve(points_.size());
    for (std::size_t i = 0; i < points_.size(); ++i) {
      double prefix = 0;
      for (std::size_t j = 1; j <= i; ++j) {
        prefix += Dist(points_[j - 1], points_[j]);
      }
      result.push_back(prefix);
    }
    return result;
  }

  // Позиция в момент времени t.
  // ТЗ 2.3: линейная интерполяция между двумя соседними точками;
  // clamp на концах (t <= t первой точки -> первая, t >= t последней
  // -> последняя); пустая траектория — std::invalid_argument.
  // Review #1 (точность):
  //   1) интерполяции НЕТ — возвращается ближайшая предыдущая точка
  //      («ступенька»);
  //   2) пустая траектория — тихий Point{} вместо ошибки: «на пустой
  //      не вызывают» — и в проде молча портит траектории.
  Point PointAtTime(double t) const {
    if (points_.empty()) {
      return Point{0, 0, 0};
    }
    for (std::size_t i = 0; i + 1 < points_.size(); ++i) {
      if (t < points_[i + 1].t) {
        return points_[i];  // ближайшая предыдущая, без интерполяции
      }
    }
    return points_.back();
  }

 private:
  static double Dist(const Point& a, const Point& b) {
    const double dx = b.x - a.x;
    const double dy = b.y - a.y;
    return std::sqrt(dx * dx + dy * dy);
  }

  std::shared_ptr<const ReferenceFrame> frame_;
  std::vector<Point> points_;  // отсортировано по t
  // TODO (ТЗ 2.2): ленивый кэш кумулятивных длин:
  //   std::unique_ptr<std::vector<double>> cumlen_cache_;
  // строится по первому запросу в CumulativeLength(), сбрасывается в
  // AddPoint и TrimBefore. Почему unique_ptr, а не shared_ptr /
  // голый vector — устный вопрос + комментарий в коде.
};

// Эталонные тесты SimpleTrajectory + TrajectoryBatch.
// Не запускать до «готово».
#include "batch_fixed.h"

#include <chrono>
#include <cstddef>
#include <iostream>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

// a) статические проверки (часть ТЗ)
static_assert(std::is_nothrow_move_constructible_v<SimpleTrajectory>);
static_assert(std::is_nothrow_move_assignable_v<SimpleTrajectory>);
static_assert(!std::is_copy_constructible_v<SimpleTrajectory>);
static_assert(!std::is_copy_assignable_v<SimpleTrajectory>);
static_assert(std::is_nothrow_move_constructible_v<TrajectoryBatch>);
static_assert(!std::is_copy_constructible_v<TrajectoryBatch>);

int main() {
  int failures = 0;

  const std::shared_ptr<const ReferenceFrame> frame =
      std::make_shared<const ReferenceFrame>(ReferenceFrame{"world"});

  // b) move-конструктор
  {
    SimpleTrajectory a(frame);
    for (int i = 0; i < 5; ++i) {
      a.AddPoint(i, i * 2.0, i * 0.5);
    }
    const std::size_t use_count_before = frame.use_count();  // 2: frame + a
    SimpleTrajectory b(std::move(a));
    const bool ok = a.Empty() && b.Size() == 5 && a.Frame() == nullptr &&
                    b.Frame() == frame.get() &&
                    frame.use_count() == use_count_before;
    if (!ok) {
      std::cerr << "FAIL b: move-конструктор (a.empty=" << a.Empty()
                << ", b.size=" << b.Size() << ", use_count="
                << frame.use_count() << ")\n";
      ++failures;
    } else {
      std::cout << "  b: move-конструктор OK (use_count "
                << use_count_before << " -> " << frame.use_count()
                << ")\n";
    }
  }

  // c) move-присваивание + self-assignment
  {
    SimpleTrajectory a(frame);
    a.AddPoint(1, 2, 3);
    SimpleTrajectory c;
    c = std::move(a);
    const bool assign_ok = c.Size() == 1 && a.Empty() && c.Frame() == frame.get();

    SimpleTrajectory d(frame);
    d.AddPoint(9, 9, 9);
    d = std::move(d);  // self-move: не падает, guard this != &other
    const bool self_ok = d.Size() == 1;

    if (!assign_ok || !self_ok) {
      std::cerr << "FAIL c: move-присваивание (assign_ok=" << assign_ok
                << ", self_ok=" << self_ok << ")\n";
      ++failures;
    } else {
      std::cout << "  c: move-присваивание + self-assignment OK\n";
    }
  }

  // d) batch: Add / ExtractAll
  {
    TrajectoryBatch batch(frame);
    for (int i = 0; i < 3; ++i) {
      SimpleTrajectory t(frame);
      t.AddPoint(i, i, i);
      batch.Add(std::move(t));
    }
    const std::size_t size_before = batch.Size();
    const auto got = batch.ExtractAll();
    const bool ok = size_before == 3 && got.size() == 3 &&
                    batch.Size() == 0 &&  // батч пустеет после ExtractAll
                    batch.ExtractAll().empty() &&
                    got[1].Size() == 1;
    if (!ok) {
      std::cerr << "FAIL d: batch (size=" << batch.Size()
                << ", got.size=" << got.size() << ")\n";
      ++failures;
    } else {
      std::cout << "  d: Add/ExtractAll OK\n";
    }
  }

  // e) vector<TrajectoryBatch> + push_back(std::move) x 1000
  {
    std::vector<TrajectoryBatch> v;
    v.reserve(1000);
    for (int i = 0; i < 1000; ++i) {
      TrajectoryBatch b(frame);
      SimpleTrajectory t(frame);
      t.AddPoint(i, 0, 0);
      b.Add(std::move(t));
      v.push_back(std::move(b));
    }
    const bool ok = v.size() == 1000 && v[999].Size() == 1;
    if (!ok) {
      std::cerr << "FAIL e: vector of batches (size=" << v.size() << ")\n";
      ++failures;
    } else {
      std::cout << "  e: vector<TrajectoryBatch> + push_back(move) x1000 OK\n";
    }
  }

  // f) PERF: move батча 10^6 точек — O(1) на вектор указателей.
  {
    TrajectoryBatch batch(frame);
    for (int i = 0; i < 1000; ++i) {
      SimpleTrajectory t(frame);
      for (int j = 0; j < 1000; ++j) {
        t.AddPoint(j, j * 1.5, j * 0.01);
      }
      batch.Add(std::move(t));
    }
    const auto t0 = std::chrono::steady_clock::now();
    TrajectoryBatch b2 = std::move(batch);
    const auto t1 = std::chrono::steady_clock::now();
    const auto us =
        std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    std::cout << "  f: move батча (10^6 точек): " << us << " us\n";
    const bool ok = b2.Size() == 1000 && batch.Size() == 0 && us < 10'000;
    if (!ok) {
      std::cerr << "FAIL f: move (b2.size=" << b2.Size()
                << ", batch.size=" << batch.Size() << ", " << us
                << " us)\n";
      ++failures;
    }
  }

  std::cout << (failures == 0 ? "ALL TESTS PASSED" : "TESTS FAILED")
            << ": " << failures << " failure(s)\n";
  return failures;
}

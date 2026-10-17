// Эталонные тесты ParseLog. Не запускать до «готово».
#include "log_parser_fixed.h"

#include <chrono>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

namespace {

struct ExpectedEvent {
  long long id;
  double t;
  double x;
  double y;
};

bool Close(double a, double b) { return std::abs(a - b) < 1e-12; }

// Разбирает ОДНУ строку: если валидна — выставляет ev, true;
// иначе false.
bool ExpectLine(const std::string& line, const ExpectedEvent* ev, int& failures,
                const char* name) {
  const ParseResult r = ParseLog({line});
  if (ev == nullptr) {
    if (r.skipped != 1 || !r.events.empty()) {
      std::cerr << "FAIL " << name << ": ожидал skipped, got events="
                << r.events.size() << ", skipped=" << r.skipped << "\n";
      ++failures;
    } else {
      std::cout << "  " << name << " OK (skipped)\n";
    }
    return r.skipped == 1 && r.events.empty();
  }
  if (r.events.size() != 1 || r.skipped != 0) {
    std::cerr << "FAIL " << name << ": events=" << r.events.size()
              << ", skipped=" << r.skipped << "\n";
    ++failures;
    return false;
  }
  const ObjectEvent& e = r.events[0];
  const bool ok = e.id == ev->id && Close(e.t, ev->t) && Close(e.x, ev->x) &&
                  Close(e.y, ev->y);
  if (!ok) {
    std::cerr << "FAIL " << name << ": got {" << e.id << ", " << e.t << ", "
              << e.x << ", " << e.y << "}\n";
    ++failures;
  } else {
    std::cout << "  " << name << " OK\n";
  }
  return ok;
}

}  // namespace

int main() {
  int failures = 0;

  // a) точность
  {
    ExpectLine("t=1.5 id=7 x=-2.0 y=3.25",
               &ExpectedEvent{7, 1.5, -2.0, 3.25}, failures, "a1");
    ExpectLine("id=7 x=-2.0 y=3.25 t=1.5",
               &ExpectedEvent{7, 1.5, -2.0, 3.25}, failures, "a2 (порядок)");
    ExpectLine("type=car t=1.5 id=7 x=0 y=0",
               &ExpectedEvent{7, 1.5, 0.0, 0.0}, failures, "a3 (доп. поле)");
    ExpectLine("t=1e1 id=1 x=0 y=0",
               &ExpectedEvent{1, 10.0, 0.0, 0.0}, failures, "a4 (1e1)");
    ExpectLine("id=1 t=1 x=0 y=0", &ExpectedEvent{1, 1.0, 0.0, 0.0},
               failures, "a5");
    ExpectLine("id=1 t=1 x=0 y=0 lane=3", &ExpectedEvent{1, 1.0, 0.0, 0.0},
               failures, "a6 (доп. поле в хвосте)");
    ExpectLine("t=abc id=1 x=0 y=0", nullptr, failures, "a7 (t=abc)");
    ExpectLine("id=1.5 t=1 x=0 y=0", nullptr, failures, "a8 (id=1.5)");
    ExpectLine("id=1 t=1 x=0 y=", nullptr, failures, "a9 (пустое y)");
    ExpectLine("id=1 t=1 x=0", nullptr, failures, "a10 (нет y)");
    ExpectLine("id=1 t=nan x=0 y=0", nullptr, failures, "a11 (nan)");
    ExpectLine("", nullptr, failures, "a12 (пустая строка)");
    ExpectLine("x=1 id=1 t=1", nullptr, failures, "a13 (нет y, мусор x=1)");
  }

  // b) смешанный лог
  {
    const std::vector<std::string> lines = {
        "t=1 id=1 x=0 y=0",
        "битая строка без равенств",
        "id=2 t=2 x=3 y=4",
    };
    const ParseResult r = ParseLog(lines);
    const bool ok = r.events.size() == 2 && r.skipped == 1 &&
                    r.events[0].id == 1 && r.events[1].id == 2;
    if (!ok) {
      std::cerr << "FAIL b: events=" << r.events.size()
                << ", skipped=" << r.skipped << "\n";
      ++failures;
    } else {
      std::cout << "  b: mixed log OK (2 events, 1 skipped)\n";
    }
  }

  // c) perf: 100 000 валидных строк
  {
    const std::size_t n = 100'000;
    std::vector<std::string> lines;
    lines.reserve(n);
    for (std::size_t i = 0; i < n; ++i) {
      lines.push_back("t=" + std::to_string(i) + " id=" + std::to_string(i) +
                      " x=" + std::to_string(i) + " y=" +
                      std::to_string(i));
    }
    const auto t0 = std::chrono::steady_clock::now();
    const ParseResult r = ParseLog(lines);
    const auto t1 = std::chrono::steady_clock::now();
    const auto ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    std::cout << "  c: " << ms << " ms, events = " << r.events.size()
              << ", skipped = " << r.skipped << "\n";
    if (r.events.size() != n || r.skipped != 0) {
      std::cerr << "FAIL c: содержимое\n";
      ++failures;
    }
    if (ms > 200) {
      std::cerr << "FAIL c: бюджет 200 мс\n";
      ++failures;
    }
  }

  std::cout << (failures == 0 ? "ALL TESTS PASSED" : "TESTS FAILED")
            << ": " << failures << " failure(s)\n";
  return failures;
}

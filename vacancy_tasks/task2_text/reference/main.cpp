// Эталонные тесты CleanLine. Не запускать до «готово».
#include "cleaner_fixed.h"

#include <chrono>
#include <iostream>
#include <string>
#include <vector>

int main() {
  int failures = 0;

  // a) точность: {вход, выход, happy, sad}
  {
    struct Case {
      std::string in;
      std::string out;
      std::size_t happy;
      std::size_t sad;
    };
    const std::vector<Case> cases = {
        {"", "", 0, 0},
        {":-)", "", 1, 0},
        {":-)))", "", 1, 0},
        {":-((", "", 0, 1},
        {":-)( ", "( ", 1, 0},
        {"hi :-) w", "hi  w", 1, 0},
        {"a:-)b:-((c", "ac", 1, 1},
        {":-", ":-", 0, 0},
        {":-~)", ":-~)", 0, 0},
        {":--)", ":--)", 0, 0},
        {")  :-", ")  :-", 0, 0},
        {":((", ":((", 0, 0},
        {"x:-)y:-)z", "xyz", 2, 0},
    };
    for (const Case& c : cases) {
      const auto [out, stats] = CleanLine(c.in);
      if (out != c.out || stats.happy != c.happy || stats.sad != c.sad) {
        std::cerr << "FAIL a: в=\"" << c.in << "\" ->=\"" << out
                  << "\" (happy=" << stats.happy << ", sad=" << stats.sad
                  << "), ожидало=\"" << c.out << "\" (happy=" << c.happy
                  << ", sad=" << c.sad << ")\n";
        ++failures;
      }
    }
    if (failures == 0) {
      std::cout << "  a: 13 точность кейсов\n";
    }
  }

  // b) perf: 100 000 символов, ~50 000 смайликов.
  {
    std::string line;
    line.reserve(100'000);
    while (line.size() < 100'000) {
      line += "ab :-) c ";  // 9 символов, 1 смайлик
    }
    // После чистки от каждого "ab :-) c " (9 символов) остаётся
    // "ab  c " (6 символов): смайлик ":-)" вырезан, пробелы на месте.
    const std::size_t patterns = line.size() / 9;
    const auto t0 = std::chrono::steady_clock::now();
    const auto [out, stats] = CleanLine(line);
    const auto t1 = std::chrono::steady_clock::now();
    const auto ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    std::cout << "  b: " << ms << " ms, happy = " << stats.happy
              << ", длина выхода = " << out.size() << '\n';
    if (stats.happy != patterns || out.size() != 6 * patterns) {
      std::cerr << "FAIL b: содержимое (happy=" << stats.happy
                << " expected " << patterns << ", out.size=" << out.size()
                << " expected " << 6 * patterns << ")\n";
      ++failures;
    }
    if (ms > 100) {
      std::cerr << "FAIL b: бюджет 100 мс (O(n^2)?)\n";
      ++failures;
    }
  }

  std::cout << (failures == 0 ? "ALL TESTS PASSED" : "TESTS FAILED")
            << ": " << failures << " failure(s)\n";
  return failures;
}

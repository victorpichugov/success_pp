#pragma once

#include <cstddef>
#include <string>

// Legacy. Функция пришла из старого текстового модуля (2021), через
// "просто работает". На ревью подняли замечания — смотрите ТЗ в main.cpp.
//
// Грамматика смайлика (ФИКСИРОВАННАЯ, не менять):
//   смайлик = подстрока ":-", сразу за которой идёт непустой пропуск
//   ОДИНАКОВЫХ скобок — либо все ")", либо все "(".
//   Удаляется МАКСИМАЛЬНЫЙ такой пропуск (вместе с ":-").
//   Всё остальное сохраняем как есть, в исходном порядке.
//
//   ":-)"     -> ""
//   ":-)))"   -> ""
//   ":-(("    -> ""
//   ":-)( "   -> "( "       (удаляется только пропуск ")" длиной 1,
//                                       "(" не часть пропуска и остаётся)
//   "hi :-) w"-> "hi  w"
//   ":-"      -> ":-"       (не за скобкой — это текст)

bool isSmileyStart(const std::string& text, std::size_t pos) {
  return text[pos] == ':' && text[pos + 1] == '-';
}

bool isFaceChar(char c) {
  return c == ')' || c == '(' || c == '~';
}

// Time: O(n) — read только движется вперёд. Space: O(1) доп.
std::string RemoveSmileys(std::string text) {
  std::size_t read = 0;
  std::size_t write = 0;
  const std::size_t n = text.size();

  while (read < n) {
    if (read + 2 < n && isSmileyStart(text, read) && isFaceChar(text[read + 2])) {
      const char paren = text[read + 2];
      read += 3;
      while (read < n && text[read] == paren) {
        ++read;
      }
    } else {
      text[write] = text[read];
      ++read;
      ++write;
    }
  }
  text.resize(write);
  return text;
}
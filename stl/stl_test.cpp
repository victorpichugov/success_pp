

#include <iostream>
#include <vector>
#include <deque>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <stack>
#include <queue>
#include <algorithm>
#include <iterator>
#include <forward_list>
#include <list>



// //  Bidirectional итераторы
// int main() {
//     std::list<int> l = {1, 2, 3, 4, 5};
//     for (auto it = l.rbegin(); it != l.rend(); ++it) {
//         std::cout << *it << " ";
//     }
//     std::cout << std::endl;
//     return 0;
// }

// // forward итераторы
// int main() {
//     std::forward_list<int> flist = {1, 2, 3, 4, 5};
//     for (auto it = flist.begin(); it != flist.end(); ++it) {
//         std::cout << *it << " ";
//     }
//     std::cout << std::endl;
//     return 0;
// }


// // output итераторы
// int main() {
//     std::ostream_iterator<int> out_it (std::cout, ", ");
//     std::vector<int> data = {1, 2, 3, 4, 5};

//     std::copy(data.begin(), data.end(), out_it);
//     return 0;
// }


// input итераторы
// int main() {
//     std::vector<int> data = {1, 2, 3, 4, 5};
//     std::istream_iterator<int> start(std::cin), end;
//     std::vector<int> numbers(start, end);

//     for (size_t n: numbers) {
//         std::cout << n << " ";
//     }
//     std::cout << std::endl;
//     return 0;
// }


// int main() {
//     std::vector<int> data = {1, 2, 3, 4, 5};
//     std::vector<int> transformData(data.size());

//     std::transform(data.begin(), data.end(), transformData.begin(),
//         [](int x){return x * 2 - 1;});

//     for (size_t d: transformData) {
//         std::cout << d << " ";
//     }
//     std::cout << std::endl;

//     return 0;
// }



// // это сниппет - встроенная функция автодополнения vs code 
// int main(int argc, char const *argv[])
// {
//     // argc - это количество аргументов argument count
//     // argv - массив строк с аргументами
//     return 0;
// }




// int main() {
//     std::vector<int> data = {1, 4, 2, 3, 2, 3, 4, 5, 5, 6};
//     std::sort(data.begin(), data.end());


//     // Удаление дубликатов
//     auto last = std::unique(data.begin(), data.end());

//     // auto - это не тип а ключевое слово!
//     // имеется ввиду, что компилятору нужно самому определить тип переменной
    
//     // std::cout << last << std::endl; // это не работает, потому что нет инициализатора
//     // auto требует тип, чтобы тип можно было вывести из режима справа
//     // auto x = 52;
//     // а это сработает, потому что справа 52 - определенный тип

//     data.erase(last, data.end());


//     for (size_t d: data) {
//         std::cout << d << " ";
//     }
//     std::cout << std::endl;

//     return 0;
// }



// int main() {
//     std::vector<int> data = {1, 2, 3, 4, 5};
//     std::transform(data.begin(), data.end(), data.begin(),
//         [](int x) { return x * x; });
//     for (int n : data) {
//         std::cout << n << " ";
//     }
//     std::cout << std::endl;
//     return 0;
// }

// int main() {
//     std::priority_queue<int> numbers;
//     numbers.push(3);
//     numbers.push(1);
//     numbers.push(4);
//     numbers.push(2);

//     // иззвлекаем элементы
//     while (!numbers.empty()) {
//         std::cout << numbers.top() << " "; // показываем элемент с наивысшим приоритетом
//         numbers.pop(); // удаляем элемент с наивысшим приоритетом
//     }
//     std::cout << std::endl;

//     return 0;
// }


// int main() {
//     std::queue<int> numbers;
//     numbers.push(1); // добавляем элементы
//     numbers.push(2);
//     numbers.push(3);

//     // извлекаем элементы
//     while (!numbers.empty()) {
//         std::cout << numbers.front() << " "; // показываем первый элемент
//         numbers.pop(); // удаляем первый элемент
//     }
//     std::cout << std::endl;

//     return 0;
// }



// int main() {
//     std::stack<int> numbers;
//     numbers.push(1); // добавляем элементы
//     numbers.push(2);
//     numbers.push(3);

//     // Извлекаем элементы
//     while (!numbers.empty()) {
//         std::cout << numbers.top() << " "; // показываем верхний элемент
//         numbers.pop(); // удаляем верхний элемент
//     }
//     std::cout << std::endl;

//     return 0;
// }


// int main() {
//     std::unordered_map<std::string, int> fruitCount = {{"apple", 2}, {"banana", 5}};
//     fruitCount["orange"] = 4; // дбавляем новую пару ключ-значение

//     // удаление элемента
//     fruitCount.erase("banana");

//     // доступ к элементам
//     std::cout << "Apples: " << fruitCount["apple"] << std::endl;

//     // итерация по unordered_map
//     for(const auto& pair : fruitCount) {
//         std::cout << pair.first << ": " << pair.second << std::endl;
//     }

//     return 0;
// }



// int main() {
//     std::unordered_set<int> numbers = {4, 1, 2, 3, 5};
//     numbers.insert(6);
//     numbers.erase(1);

//     if (numbers.find(4) != numbers.end()) {
//         std::cout << "4 is in the unordered_set" << std::endl;
//     }

//     for (int num: numbers) {
//         std::cout << num << " ";
//     }
//     std::cout << std::endl;
//     return 0;
// }

// int main() {
//     std::multimap<std::string, int> fruitCount = {{"apple", 2}, {"apple", 3}, {"banana", 5}};
//     fruitCount.insert({"apple", 1}); // добавляем еще одно значение для ключа "apple"
    
//     auto range = fruitCount.equal_range("apple");
//     for(auto it = range.first; it != range.second; ++it) {
//         std::cout << it->first << ": " << it->second << std::endl;
//     }

//     return 0;
// }

// int main() {
//     std::map<std::string, int> fruitCount = {{"apple", 2}, {"banana", 5}};
//     fruitCount["orange"] = 4;
    
//     std::cout << "Apples: " << fruitCount["apple"] << std::endl;

//     for (const auto& pair: fruitCount) {
//         std::cout << pair.first << ": " << pair.second << std::endl;
//     }

//     return 0;
// }


// int main() {
//     std::multiset<int> numbers = {5, 2, 1, 3, 2};
//     numbers.insert(2);

//     for (int num: numbers) {
//         std::cout << num << " ";    
//     }
//     std::cout << std::endl;
//     return 0;
// }



// int main() {
//     std::set<int> numbers = {4, 2, 1, 3, 5};
//     numbers.insert(6);
//     numbers.insert(3);

//     if (numbers.find(4) != numbers.end()) {
//         std::cout << "4 is in the set " << std::endl;
//     }


//     for (int num: numbers) {
//         std::cout << num << " ";    
//     }
//     std::cout << std::endl;
//     return 0;
// }




// int main() {
//     std::deque<int> numbers = {2, 3, 4};
//     numbers.push_front(1); // добавляем элемент в начало
//     numbers.push_back(5);


//     numbers.pop_front();


//     for (int num: numbers) {
//         std::cout << num << " ";    
//     }
//     std::cout << std::endl;
//     return 0;
// }
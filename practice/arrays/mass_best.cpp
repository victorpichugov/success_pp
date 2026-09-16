#include <iostream>
#include <limits>
#include <utility>   // Для std::pair
#include <cstdint>
#include <stdexcept>

// Возвращает пару: {успех, значение}. 
// Если успех == false, то значение невалидно.
std::pair<bool, int64_t> find_maximum(std::istream& input_stream, size_t expected_count) 
{
    if (expected_count == 0) {
        return {false, 0}; // false означает "данных нет"
    }

    int64_t current_max = std::numeric_limits<int64_t>::min();
    bool has_valid_data = false;

    for (size_t i = 0; i < expected_count; ++i) {
        int64_t value;
        
        if (input_stream >> value) {
            if (value > current_max) {
                current_max = value;
            }
            has_valid_data = true;
        } else {
            throw std::invalid_argument("Ошибка чтения данных.");
        }
    }

    return {has_valid_data, current_max};
}

int main() 
{
    std::istream& data_source = std::cin; 

    size_t n;
    if (!(data_source >> n)) {
        std::cerr << "Ошибка: не удалось прочитать количество элементов.\n";
        return 1;
    }

    try {
        auto result = find_maximum(data_source, n);

        // Проверяем первый элемент пары (флаг успеха)
        if (result.first) {
            std::cout << "Максимальный элемент: " << result.second << "\n";
        } else {
            std::cout << "Данные отсутствуют.\n";
        }
    } 
    catch (const std::invalid_argument& e) {
        std::cerr << "Критическая ошибка: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
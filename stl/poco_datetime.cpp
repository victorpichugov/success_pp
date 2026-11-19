#include <Poco/DateTime.h>
#include <Poco/DateTimeFormat.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/Timestamp.h>
#include <Poco/Timespan.h>  // ← добавили правильный заголовок
#include <iostream>

using Poco::DateTime;
using Poco::DateTimeFormatter;
using Poco::DateTimeFormat;
using Poco::Timestamp;
using Poco::Timespan;  // ← правильное имя класса

int main() {
    // 1. Текущее время
    DateTime now;
    std::cout << "=== Текущее время ===" << std::endl;
    std::cout << "Год:    " << now.year() << std::endl;
    std::cout << "Месяц:  " << now.month() << std::endl;
    std::cout << "День:   " << now.day() << std::endl;
    std::cout << "Час:    " << now.hour() << std::endl;
    std::cout << "Минута: " << now.minute() << std::endl;
    std::cout << "Сек:    " << now.second() << std::endl;
    
    // 2. Красиво отформатированная строка
    std::cout << "\n=== Форматированный вывод ===" << std::endl;
    std::cout << DateTimeFormatter::format(now, DateTimeFormat::SORTABLE_FORMAT) << std::endl;
    std::cout << DateTimeFormatter::format(now, "%Y-%m-%d %H:%M:%S") << std::endl;
    std::cout << DateTimeFormatter::format(now, "%A, %d %B %Y") << std::endl;
    
    // 3. Создание конкретной даты
    DateTime birthday(1990, 5, 15, 12, 30, 0); // год, месяц, день, час, мин, сек
    std::cout << "\n=== Дата рождения ===" << std::endl;
    std::cout << DateTimeFormatter::format(birthday, "%d.%m.%Y %H:%M") << std::endl;
    
    // 4. Разница между датами (Timespan, а не TimeSpan!)
    Timestamp ts1 = now.timestamp();
    Timestamp ts2 = birthday.timestamp();
    Timespan diff = ts1 - ts2;  // ← исправлено
    std::cout << "\n=== Прошло с даты рождения ===" << std::endl;
    std::cout << "Дней:  " << diff.days() << std::endl;
    std::cout << "Часов: " << diff.hours() << std::endl;
    
    return 0;
}
#include <ace/Reactor.h>
#include <iostream>

class MyTimerHandler : public ACE_Event_Handler {
private: 
    int tick_count = 0;
public:
    // Этот метод вызывается реактором при срабатывании таймера
    int handle_timeout(const ACE_Time_Value &current_time, const void *act = nullptr) override {
        tick_count++;
        std::cout << "[Таймер] Срабатывания №" << tick_count << " (время: " << current_time.sec() << " сек.)" << std::endl;

        // Останавливаем реактор после 5 срабатывания для демонстрации корректного завершения
        if (tick_count >= 5) {
            std::cout << "[Таймер] Достигнут лимит. Останавливаем цикл событий" << std::endl;
            ACE_Reactor::instance()->end_reactor_event_loop();
            return -1; // Возврат -1 говорит реактору удалить этот таймер
        }

        return 0; // Возврат 0 означает, что таймер должен работать
    }
};



int main() {
    std::cout << "=== Запуск ACE Reactor Timer ===" << std::endl;
    MyTimerHandler timer_handler;
    ACE_Time_Value interval(1); // Интервал 1 секунда

    // Планируем таймер: 
    // 1. Указатель на обработчик
    // 2. Пользовательские данные (act) - нам не нужны, поэтому nullptr
    // 3. Время до первого срабатывания (ACE_Time_Value::zero = немедленно)
    // 4. Интервал между срабатываниями

    long timer_id = ACE_Reactor::instance()->schedule_timer(
        &timer_handler,
        nullptr,
        ACE_Time_Value::zero,
        interval
    );

    if (timer_id == -1) {
        std::cerr << "Ошибка: не удалось запланировать таймер!" << std::endl;
        return 1;
    }

    std::cout << "Реактор запущен. Ожидание событий..." << std::endl;

    ACE_Reactor::instance()->run_reactor_event_loop();

    std::cout << "=== Программа успешно завершена ===" << std::endl;
    return 0;
}
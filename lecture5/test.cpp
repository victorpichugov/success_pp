
// Классы, перегрузка операторов

class String
{
public:
    String(const char* str, int size){ // конструктор
        str_ = new char[size]; // выделение памяти

    }
    ~String() {delete[] str_;} // деструктор
private:
    char* str_;
    int size_;
};

int main(){
    String s = String("abc", 3);
    String s1 = s; // конструктор копирования пока нет => runtime error

    // Он сгенерирует констурктор копирования, но там буду тупо поля
    // Дальше 2 раза будет delete[]
}

// Конструктор копирования 
String(const String str){
    ...
}

// Если я хочу написать 2 Конструктор копирования с не константными str
// Да могу!


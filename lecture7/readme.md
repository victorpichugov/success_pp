Лекция 7 Наследование

2) Поиск имен при наследовании

2а) Сокрытие имен наследника

Base f(int x)

^
|

Derived f()

Derived d;
d.f() // ошибка компиляции

2б) Явный вызов имени предка


```cpp
class Base {
    int b;
    void f();
};

class Derived : public Base {
    int d;
    void f(int x);
};

Derived d;
хочу обратиться к f() который в Base, тогда нужно написать явный квалификатор

d.Base::f(5);

А если наследование приватное?
Потому что если приватное, то тем более ошибка компиляции потому что я не имею права
вызывать метод родителя

Ошибка: Не видна область/ Не доступна(не имеешь права вызывать inacceseble) 

В теле класса Derived можно написать using Base::f;

2в)

struct Base {
    void f();
}


Каверзные вопросы на собеседовании:
если не написать public, private или protected, то что будет?
по умолчанию public, как и поля
struct Derived: Base {}

по умолчанию private, как и поля    
class Derived: Base {}


struct Base {
    void f() {}
}

struct Derived: Base {
    private:
        void f(int) {}
}

Derived d;
d.f(); // можно ли так вызывать? Нет нельзя! Ошибка компиляции
Потому что проверка доступа происходит после поиска имен
Сначала компилятор решает все неоднозначности с вызывом и только потом проверяет
имеете ли вы право это вызывать

Сначала решить какие из f() мне подходит и только потом public, private  и тд...

2г) 
2 уровневая система наследования

struct Granny {
    int a;
};

struct Mom: private Granny {
    int b;
};

struct Son: public Mom {
    int c;
};

Son s;
s.c; // ok
s.b; // ok
s.a; // error
s.Mom::b // ok
s.Mom.Granny::a // error
s.Granny::a // error


пусть 
struct Son: public Mom {
    int c;
    void f() {
        Mom m; // ok
        Granny g; // ошибка компиляции, потому что внутри Son
    }
    ...
}


пусть хочу объявить в сыне функцию, которая не объявляет бабушку, а
принимает бабушку - ошибка компиляции  
struct Son: public Mom {
    int c;
    void f(Granny g) {
        Mom m; // ok
    }
    ...
}

так тоже нельзя
struct Son: public Mom {
    int c;
    void f(Granny& g) {
        Mom m; // ok
    }
    ...
}

и так нельзя
struct Son: public Mom {
    int c;
    void f(Granny* g) {
        Mom m; // ok
    }
    ...
}

потому что после того, как я вошел в s я не имею права писать Granny, потому что
в самом слове s Granny - это запретное слово

Что же делать? Искать Granny в глобальной области видимости, в области видимости Son
Granny не будет видно

struct Son: public Mom {
    int c;
    void f(::Granny& g) {
        Mom m; // ok
    }
    ...
}

Следующий пример:

friend - это тот, кому можно тоже, что можно членам
struct Granny {
    int a;
    friend class Son;
};

struct Mom: private Granny {
    int b;
};

struct Son: public Mom {
    int c;
};

Это не поможет!! Проблема возникает тут s.Granny
                                         ^

А если так? То можно убрать :: в Son? Так можно! Потому что можно s.Granny этот
запрет был наложен мамой, а не бабушкой, теперь мама разрешила общаться с бабушкой

struct Granny {
    int a;
    friend class Son;
};

struct Mom: private Granny {
    int b;
    friend class Son;
};

struct Son: public Mom {
    int c;
    void f(::Granny& g) {
        Mom m; // ok
    }
};

2д) 

struct Granny {
    protected:
        int a;
};

struct Mom: public Granny {
    int b;
    friend void f(); // Мама разрешила для f() доступ ко всему
};

struct Son: public Mom {
    int c;
};

В глобальной области видимости
void f() {
    Mom m;
    m.a; // Вопрос, можно ли так? Да! Потому что друзья имеют доступ ко всему, 
    // до чего имеет доступ сама мама. Но дружба не транзитивна!
}

Но дружба не транзитивна! Если мама дружит с кем то другим, то это не означает, что f
может и к другу получить доступ

3) Порядок вызова конструкторов и деструкторов при наследовании

Скомпилируется ли это?
Если да, то упадет ли это?
Если нет, то утечка памяти будет?

struct Granny {
    int a;
    Granny(int a):a(a) {}
};

struct Mom: public Granny {
    int b;
    Mom(int b):b(b) {} // из-за этой строки не скомпилируется 
};

Что такое мама? Это бабушка к которой приставлено еще int в памяти
Мама - это все, что было у бабушки а потом то что относиться к маме
sizeof(Mom) = 8 // размер 2 интов int (бабушки) + int (личный)

Если бы не было конструктора Granny(int a):a(a) {} то все бы работало
потому что когда не конструктора компилятор сам напишет его по умолчанию за меня с
параметрами по умолчанию

Но что если я не хочу менять бабушку но хочу уметь конструировать маму от 1 параметра

для этого нужно написать:
struct Mom: public Granny {
    int b;
    Mom(int b):Granny(...):b(b) {} // из-за этой строки не скомпилируется 
    Mom(int a, int b): a(a), b(b) {} // так делать нельзя, можно инициализировать только 
    // свои поля
};

// Чем отличается функтор от внешнего конструктора?


Допустим у мамы есть конструктор по умолчанию 
struct Mom: public Granny {
    int b;
    Mom(int b):Granny(0):b(0) {} 
};

Делаем сына
Можно ли написать так?
struct Son: Mom{
    int c;
    Son(int c):Granny(0) c(c) {} // ошибка компиляции Granny is not a directed Base of Son
    // так можно только прямых предков инициализировать
}

Деструкторы вызываются в обратном порядке конструкторов


4) Множественное наследование - multiple inheritance

Пример: параллелограмм, ромб и прямоугольник, квадрат

параллелограмм
прямоугольник - наследник параллелограмм
ромб - наследник параллелограмм
квадрат - наследник прямоугольник
квадрат - наследник ромб

struct Rhombus {
    int z;
}

struct Rectangle {
    int y;
}

struct Parallelogram {
    int x;
}

struct Square: Rhombus, Rectangle{
    int t;
};

Чему будет равен размер Square?
sizeof(Square) = (P)int + (Rh)int + (P)int + (Rh)int + (S)int

Следующая проблема:
4а) Square s;
s.x; // неоднозначность потому что 2 x от Rectangle и от Rhombus
s.Rhombus::x;
s.Rectangle::x;
s.Parallelogram::x; // тоже неодозначность потому что их 2

тоже самое и с функциями

Это проблема называется Diamond problem проблема ромбовидного наследования
из за этого считается, что множественное наследование не очень хороший метод
проектирования, 

Разговоры в яндексе: может разрешим множественное наследование? 
Ага, сначала разрешим множественное наследование потом goto, а потом становимся фашистами

Следующий пример:

struct Rectangle: private Parallelogram{}

struct Rhombus: public Parallelogram{}

struct Square: public Rhombus, Rectangle {
    int x;
}

Square s;
s.x; что будет?

4б) 

Granny <-
^       |
|       |
Mom     |
^       |
|       |
        |
Son -----


Скомпилируется ли это? Да! Но это warning Будет 2 бабушки: одна от мамы другая напрямую 
struct Granny {}
struct Mom: {}
struct Son: {
    public Mom,
    public Granny {};
}

Son s;
s.x Ошибка компиляции

На самом деле выглядит так

Granny(x)        Granny(x) <-
^                           |
|                           |
Mom                         |
^                           |
|                           |
                            |
Son ------------------------

Son s;
s.Granny::x

5) Виртуальное наследование != Виртуальные функции

Как сказать компилятору: не создавай копии одного и теже базового класса, если мы разными
путями до него наследовались

Одно и тоже
struct Rectangle: virtual public Parallelogram {}
struct Rectangle: virtual  Parallelogram {}
struct Rhombus: virtual Parallelogram{}

тогда компилятор обязан создать только один параллелограмм и всякий раз компилятор за этим 
следит если дальше будет иерархия

А что если:
struct Rectangle: virtual Parallelogram {}
struct Rhombus: Parallelogram{}

тогда будет 2 Parallelogram
Все виртуальные соединятся в 1, а все не виртуальные создаст свою копию

как происходит поиск имен?

struct Rectangle: virtual Parallelogram {}
struct Rhombus: Parallelogram{}

будет неоднозначность

пусть есть классы W V A B D

W (int y)         V (int x)        W (int y)
^                 ^                ^
|                 | (вирт)         | (вирт)
(не вирт)         |                |
A --------------------(не вирт)--> B
^                                  ^
|                                  |
D-----------------------------------(не вирт)

Что будет D d;
d.x; // скомпилируется
d.y; // ошибка

Пример из стандарта Глава 10

6) Приведение типов между наследниками

пусть у нас есть Base, Derived

Виртуальное наследование и его не очень любят и  очень дооолго по времени и компилятор в памяти хранит этот граф

Base b;
Derived d;
b = d; // можно но не желательно. Это называется срезка из копирования
d = b; // нельзя

А можно так?
Base& b = d; да так можно, но через b можно обратиться только к той части d которая относится к Base, но и к Derived тоже можно, как?

static_case<Direved&>(b);

но рекомендуется делать через dynamic_case

= - это инициализация а не присваивание!!

А можно так?
Base* b = d; да, можно

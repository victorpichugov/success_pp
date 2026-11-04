Лекция 14 Value categories и move семантика

(лекция восстановлена по-plan'у: темы прямо обещаны в 13-й и 5-й)

на прошлой лекции осталось два «неправильно»:
1) traits::construct(alloc, newarr + i, arr[i]); // «это неправильно, но для 2007 года норм, надо делать через move»
2) template<class ... Args> void emplace_back(const Args& ... args); // «и это неправильно. суть в move семантике»

почему неправильно:
arr[i] — lvalue => конструктор КОПИРОВАНИЯ. relocate вектора = перекопировать всё.
у emplace_back все аргументы const& => v.emplace_back(10, 'x') строит строку-аргумент копией,
а rvalue (string("abc")) в const& возьмут и НЕ двиганут.

чиним так:
traits::construct(alloc, newarr + i, std::move(arr[i]));
template<class... Args> void emplace_back(Args&&... args)  // БЕЗ const!!
    traits::construct(alloc, arr + sz, std::forward<Args>(args)...);

вопрос: а если move бросит исключение при реаллокации? → noexcept (см. 11-ю)

-------------------------

value categories

тест «можно ли взять &»:
int a = 1; &a - да (lvalue)
&(a + b) - НЕТ, lvalue required as unary '&' operand
&make() - нет (prvalue)

тест «можно ли присвоить»:
a = 5 - да
(a + b) = 5 - нет
3 + 4 = 5 - ошибка: левый операнд = обязан быть lvalue (это с 6-й лекции!)

наша каша с 6-й: c1 + c2 = 5 КОМПИЛИРУЕТСЯ. потому что c1 + c2 — временный объект,
а у нашего класса operator= есть, он примет *this. Мы лечили через const-возврат:
const C operator+(...) → теперь присваивать не во что.
в C++11 есть способ красивее: квалификаторы у функции
void f() &;      // только для lvalue
void g() &&;     // только для rvalue
c.g()  // ошибка: passing 'C' as 'this' argument discards qualifiers
std::move(c).g() // ок

это rvalue: то, что умрёт в конце выражения (временный объект)
это lvalue: то, у чего есть имя/адрес и живёт дольше выражения

дерево:
        expression
        /        \
   glvalue      rvalue
   /   \        /    \
lvalue  xvalue prvalue
              (xvalue = «лвенькая ссылка на доживающее» : T&&, std::move(x), p->f() &&)

xvalue — это и glvalue (есть адрес), и rvalue (можно двигать). отсюда и имя eXpiring value.

правила:
- у rvalue нельзя взять &
- const T& продлевает жизнь временному:
  const std::string& s = "abc";  // строка живёт до конца области видимости s !!!
  std::string& s = "abc";        // не скомпилируется
  почему: ссылка владеет, объект живёт до конца области (std::string tmp = Get(); const& x = tmp; ...)
  КАВЕРЗА: string_view на временную строку = висячая
- встроенным присвоить rvalue нельзя, своим классам — можно (см. 6-ю)

-------------------------

T&& rvalue-ссылка

int a = 5;
int&& r = 5;   // ок
int&& r2 = a;  // НЕТ: cannot bind rvalue reference of type 'int&&' to lvalue

std::move НИЧЕГО НЕ ДВИГАЕТ. это каст:
template<class T> T&& my_move(T& x) { return static_cast<T&&>(x); }
он превращает lvalue в rvalue (xvalue), чтобы overload resolution выбрала конструктор от T&&
само перемещение — это МОЙ конструктор T(T&&), а не std::move.

std::forward сохраняет категорию: lvalue → lvalue, rvalue → rvalue.
forward обязателен в шаблоне, у него явный шаблонный параметр: std::forward<T>(x)
вне шаблона forward не работает: no matching function for call to 'forward(int&)'

reference collapsing (4 комбинации):
X& &   -> X&
X& &&  -> X&     ← самое главное: & && схлопывается в &
X&& &  -> X&
X&& && -> X&&    ← единственный случай &&
почему `X& &&` → `X&`: иначе T&& при T = int& дало бы int& &&, а ссылки на ссылки нелегальны,
стандарт решил схлопывать. Отсюда: T&& в шаблоне с выводом типа = универсальная ссылка.
template<class T> void f(T&& x):
  f(a)  → T = int&  → int& && → int&
  f(5)  → T = int   → int&&
у НЕшаблонного T&& (void g(int&&)) универсальной ссылки нет.

КАВЕРЗА: std::move(const T&) → const T&& → двигатель не подходит → КОНСТРУКТОР КОПИРОВАНИЯ.
std::move от const-объекта не двигает НИЧЕГО.
std::move встроенных (int, double) = копия.

-------------------------

move конструктор руками (String из 5-й лекции)

String(String&& o) noexcept : str(o.str), size(o.size) {
    o.str = nullptr; o.size = 0;      // источник обнулить ОБЯЗАТЕЛЬНО, иначе двойное delete[]
}
String& operator=(String&& o) noexcept {
    if (this != &o) {
        delete[] str;                 // 1) освободить СВОЁ
        str = o.str; size = o.size;   // 2) украсть ЧУЖОЕ
        o.str = nullptr; o.size = 0;  // 3) обнулить источник
    }
    return *this;
}
порядок именно такой: своё → чужое → обнулить. наоборот = use-after-free при self-move.
проверка this == &o обязательна (как проверка &s == this из 5-й).

на vector (из 13-й): двигать 3 указателя (arr/sz/cap) + обнулить источник. O(1).
move контейнера двигает БУФЕР, а НЕ элементы: при b = std::move(a) ни одного move-конструктора
элемента не вызывается (проверил: copies=0 moves=0 на vector<P> из 1000 элементов).

swap — самый дешёвый move: 3 обмена указателями.
swap у vector из 13-й был написан «T t = x; x = y; y = t;» — это 1 копия + 2 присваивания,
через move: 1 конструктор + 2 move-присваивания.

-------------------------

noexcept на move — ПОЧЕМУ КРИТИЧНО

vector при реаллокации двигает элементы ТОЛЬКО если move не бросает (strong guarantee, 11-я):
условие — std::is_nothrow_move_constructible_v<T> (в libstdc++ __uninitialized_move_if_noexcept_a).
не noexcept → перенос КОПИЕЙ (ради «всё или ничего»).

эксперимент (16 push_back, ёмкости 1,2,4,8 → переносим 1+2+4+8=15):
  move noexcept     : copies=0  moves=31
  move без noexcept : copies=15 moves=16
т.е. снятие noexcept = +15 копий. это и есть ответ ревьюера «relocation → copy».

копия heavy-объекта = 24 МБ на кадр (vacancy_tasks/task4_batch).

КАВЕРЗА: копирование = delete + move БЕЗ noexcept → vector не компилируется
(static assertion failed: result type must be constructible from value type of input range).
КАВЕРЗА: move-only класс со своим деструктором в C++11 не переезжает вообще (move удалён).

-------------------------

moved-from контракт

после std::move(x) объект x: «валидное, но неопределённое состояние».
гарантировано: деструктор, присваивание (в т.ч. новое), swap, size/empty для контейнеров.
НЕЛЬЗЯ: использовать значение, сравнивать с прежним, считать что пусто (для не-контейнеров).
x = std::move(x) — стандарт ничего не обещает (для своих классов guard this != &o),
у std-контейнеров самоприсваивание не падает.
кто отвечает за контракт — АВТОР КЛАССА (ответ ревьюеру): он пишет move и обязан оставить валидным.

swap-идиома: strong guarantee через «сделал копию → noexcept swap» (copy-and-swap, 11-я).

-------------------------

правило пяти (обещано на 5-й: «В C++ 11 вместо правила 3 есть правило 5»)

деструктор, копирование (ctor + =), перемещение (ctor + =) — либо все, либо никто.
когда move НЕ генерируется:
- объявлено своё копирование (ctor или operator=) — в т.ч. = delete → move нет
- C++11: объявлен свой деструктор → move = delete (двигается КОПИЕЙ!)
- C++17+: свой деструктор больше не убивает move (но is_nothrow_move_constructible = false)
лечится: продефолтить все пять (X() = default; X(const X&) = default; X(X&&) noexcept = default; ...)
= delete — запретить; = default — «хочу как компилятор, тогда генерируй».
правило нуля: свои ctor/dtor не пишем вообще, всё через RAII/умные указатели → на 15-й лекции.

-------------------------

применение

Add(T x) { v_ = std::move(x); }   // by value + move
- const& : всегда копия, rvalue не двигается
- T&&    : нельзя передать lvalue без std::move (Add(t) — ошибка компиляции)
- by value: максимум один move, принимает и lvalue, и rvalue
цифры (счётчики): by value lvalue = 1 копия + 1 move; by value rvalue = 1 move; const& = 1 копия всегда.

push_back(std::move(obj)) — двигает в контейнер.
возврат по значению: RVO/NRVO — копий нет вообще (проверил: plain return local → copies=0 moves=0).
НЕ ПИСАТЬ return std::move(local) — это запрещает NRVO (warning: -Wpessimizing-move, moves=1).
std::move в списке инициализации (5-я): C(std::shared_ptr<const Frame> f) : frame_(std::move(f)) {}
вернуть by value, а не const& (кэш): «вернуть by value, а не const&, если есть кэш» (вопрос ревьюера)
— батч отдаёт владение, consumer забирает; раздача ссылок = два владельца, никто не уверен.

ExtractAll() { return trajs_; } — КОМПИЛЯЕТСЯ и двигает (локальный объект return = implicit move).
{ return std::move(trajs_); } — тоже компилируется.
разница по факту в gcc 11 (проверил): plain возвращает vector c прежним размером (NRVO: возврат
происходит прямо в приёмник, move не происходит, поле не обнуляется), а std::move строит
в приёмник через move-конструктор → поле пустеет. Т.е. «пустеет» надо делать ЯВНЫМ move,
если батч должен освободиться.

3 вопросов на экзамене

1) vector<T> v; для 4 итераций v.emplace_back(); capacity растёт 1,2,4.
   Сколько конструкторов/деструкторов? (T ctor/dtor/copy/move считаем)
   → 7 конструкторов (4 T() + 3 move), 7 деструкторов. С reserve(4): 4 и 4.
2) 16 push_back(T()) без reserve: move noexcept → 47 конструкторов (16 + 31 move), 47 деструкторов;
   снять noexcept → 62 (31 копия вместо 31 move + 15... пересчитать на доске: 16 + 15 копий переноса
   + 31 конструкторов push_back...).
3) move-only класс (копирование = delete, move есть, БЕЗ noexcept) в vector<T>::push_back:
   компилируется? → НЕТ (статический assert про constructible). Поставить noexcept → компилируется.

бонус (из 13-й): vector<T> v; delete[] &(v[0]); → Runtime error — память принадлежала аллокатору.

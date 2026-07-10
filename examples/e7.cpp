// clang-format off

// e7.cpp — datatype with inherited base classes
//
// Syntax:  datatype(TypeName, Base1, Base2, …, (Variant, Fields…), …)
//
// All arguments before the first parenthesised variant tuple are treated as
// base-class specifiers and forwarded to the struct's inheritance list.
// Access specifiers (public, protected, virtual) work as normal.
// Up to 8 base classes are supported.

#include "../datatype.hpp"
#include <cassert>
#include <cstdio>

//─────────────────────────────────────────────────────────────────────────────
// 1. Single base class
//─────────────────────────────────────────────────────────────────────────────

struct Named { const char* name = "unnamed"; };

datatype(Animal, Named,
    (Cat, int /*lives*/),
    (Dog, int /*tricks*/)
);

static void test_single_base() {
    auto c = Animal::Cat(9);
    c.name = "whiskers";
    assert(c.name[0] == 'w');
    assert(matches(c, Cat));

    int v = match(c, {
        of(Cat, lives)  return lives;
        of(Dog, tricks) return tricks;
    });
    assert(v == 9);
}

//─────────────────────────────────────────────────────────────────────────────
// 2. Two base classes
//─────────────────────────────────────────────────────────────────────────────

struct Colored { int color = 0; };
struct Weighted { float weight = 0.f; };

datatype(Fruit, Colored, Weighted,
    (Apple, int /*variety*/),
    (Banana, int /*length*/)
);

static void test_two_bases() {
    auto a = Fruit::Apple(3);
    a.color  = 0xFF0000;
    a.weight = 0.2f;
    assert(a.color == 0xFF0000);
    assert(a.weight == 0.2f);

    int v = match(a, {
        of(Apple,  x) { return x; }
        of(Banana, x) { return x; }
    });
    assert(v == 3);
}

//─────────────────────────────────────────────────────────────────────────────
// 3. Three base classes
//─────────────────────────────────────────────────────────────────────────────

struct HasId   { int id   = 0; };
struct HasAge  { int age  = 0; };
struct HasRank { int rank = 0; };

datatype(Entity, HasId, HasAge, HasRank,
    (Player, int /*score*/),
    (Npc,    int /*level*/)
);

static void test_three_bases() {
    auto p = Entity::Player(1000);
    p.id   = 7;
    p.age  = 25;
    p.rank = 3;
    assert(p.id == 7 && p.age == 25 && p.rank == 3);

    int s = match(p, {
        of(Player, score) { return score; }
        of(Npc,    level) { return level; }
    });
    assert(s == 1000);
}

//─────────────────────────────────────────────────────────────────────────────
// 4. Access specifier (explicit public)
//─────────────────────────────────────────────────────────────────────────────

struct Base { int x = 42; };

datatype(Derived, public Base,
    (A, int),
    (B, int)
);

static void test_access_specifier() {
    auto a = Derived::A(10);
    assert(a.x == 42);
}

//─────────────────────────────────────────────────────────────────────────────
// 5. No base class — existing syntax unchanged
//─────────────────────────────────────────────────────────────────────────────

datatype(Plain, (X, int), (Y, int));

static void test_no_base() {
    auto p = Plain::X(99);
    assert(matches(p, X));
    assert(match(p, { of(X, v) { return v; } of(Y, v) { return v; } }) == 99);
}

//─────────────────────────────────────────────────────────────────────────────
// 6. Template datatype with base
//─────────────────────────────────────────────────────────────────────────────

template <typename T> struct Container { int size = 0; };

template <typename T>
datatype(Stack, Container<T>,
    (Empty),
    (Cons, T, int /*next index*/)
);

static void test_template_base() {
    Stack<int> s = Stack<int>::Cons(5, -1);
    s.size = 1;
    assert(s.size == 1);

    int v = match(s, {
        of(Empty)     { return 0; }
        of(Cons, h, _){ return h; }
    });
    assert(v == 5);
}

//─────────────────────────────────────────────────────────────────────────────

int main() {
    test_single_base();       puts("single base:       OK");
    test_two_bases();         puts("two bases:         OK");
    test_three_bases();       puts("three bases:       OK");
    test_access_specifier();  puts("access specifier:  OK");
    test_no_base();           puts("no base (compat):  OK");
    test_template_base();     puts("template base:     OK");
    puts("all tests passed");
}

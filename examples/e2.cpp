// clang-format off

// e2.cpp — template (polymorphic) datatypes
//
// Demonstrates:
//   1. `template <typename T>` directly above datatype() — works transparently
//   2. Multi-parameter template datatypes
//   3. Template datatypes used with match / matches / if_let
//   4. Static assert on generated layout

#include <cstdio>
#include <type_traits>
#include "../datatype.hpp"

// ── Generic Box<T> ───────────────────────────────────────────────────────────

template <typename T>
datatype(Box,
    (Full, T),
    (Empty)
);

// ── Generic Pair<A, B> ───────────────────────────────────────────────────────

template <typename A, typename B>
datatype(Pair,
    (MkPair, A, B)
);

// ── Generic Either<L, R> ─────────────────────────────────────────────────────

template <typename L, typename R>
datatype(Either,
    (Left,  L),
    (Right, R)
);

// ── Generic list node (singly-linked, pointer-based) ────────────────────────

template <typename T>
struct List;

template <typename T>
datatype(List,
    (Cons, T, List<T>*),
    (Nil)
);

template <typename T>
int list_length(List<T> l) {
    return match(l, {
        of(Cons, _, tail) { return 1 + list_length(*tail); }
        of(Nil)           { return 0; }
    });
}

// ── Static-layout assertions ─────────────────────────────────────────────────

static_assert(Box<int>::_variant_count  == 2);
static_assert(Pair<int,float>::_variant_count == 1);
static_assert(Either<int,float>::_variant_count == 2);

static_assert(std::is_same_v<decltype(Box<int>::Full(1)),  Box<int>>);
static_assert(std::is_same_v<decltype(Box<int>::Empty()),  Box<int>>);
static_assert(std::is_same_v<decltype(Pair<int,float>::MkPair(1, 2.f)), Pair<int,float>>);

// ── main ─────────────────────────────────────────────────────────────────────

int main() {
    // Box<int>
    Box<int> full  = Box<int>::Full(42);
    Box<int> empty = Box<int>::Empty();

    match(full) {
        of(Full, v)  { printf("Full(%d)\n", v); }   // Full(42)
        of(Empty)    { printf("Empty\n"); }
    }
    match(empty) {
        of(Full, v)  { printf("Full(%d)\n", v); }
        of(Empty)    { printf("Empty\n"); }           // Empty
    }

    // match returning a value
    int x = match(full, {
        of(Full, v)  { return v * 2; }
        of(Empty)    { return -1;    }
    });
    printf("full*2 = %d\n", x);   // 84

    // Box<float>
    Box<float> bf = Box<float>::Full(3.14f);
    if_let(bf, Full, fv) {
        printf("float box = %g\n", fv);  // 3.14
    }

    // Pair<int, float>
    Pair<int,float> p = Pair<int,float>::MkPair(7, 2.5f);
    match(p) {
        of(MkPair, a, b) { printf("pair (%d, %g)\n", a, b); }  // (7, 2.5)
    }

    // Either<int, const char*>
    Either<int, const char*> el = Either<int,const char*>::Left(10);
    Either<int, const char*> er = Either<int,const char*>::Right("hello");

    auto describe = [](Either<int,const char*> e) {
        match(e) {
            of(Left,  v) { printf("Left(%d)\n",  v); }
            of(Right, v) { printf("Right(%s)\n", v); }
        }
    };
    describe(el);   // Left(10)
    describe(er);   // Right(hello)

    // List<int>
    List<int> nil  = List<int>::Nil();
    List<int> n1   = List<int>::Cons(30, &nil);
    List<int> n2   = List<int>::Cons(20, &n1);
    List<int> n3   = List<int>::Cons(10, &n2);

    printf("list length = %d\n", list_length(n3));  // 3

    // matches on a template datatype
    printf("is Full:  %d\n", matches(full,  Full));   // 1
    printf("is Empty: %d\n", matches(empty, Full));   // 0
}

// clang-format off

// e6.cpp — GADTs via template<> + datatype()
//
// Demonstrates:
//   1. Forward-declaring a type parameter and specialising with template<>
//   2. Cross-type constructor references (IsZero, And connect int/bool trees)
//   3. Per-specialisation exhaustiveness — each eval<T> only matches reachable arms
//   4. Type-safe construction: Expr<bool>::Num() does not exist at compile time
//   5. Mutually recursive eval across two separate specialisations

#include <cassert>
#include <cstdio>
#include "../datatype.hpp"

//=============================================================================
// 1.  Typed expression AST
//
//   Expr<int>  — integer-producing nodes
//   Expr<bool> — boolean-producing nodes (IsZero takes an Expr<int>* operand)
//=============================================================================

// Give the template a single name so match.hpp's of() bindings work uniformly.
// Each specialisation is an independent tagged union — no shared state.

template <typename A> struct Expr;

template <>
datatype(Expr<int>,
    (Num,  int),
    (Add,  Expr<int>*, Expr<int>*),
    (Mul,  Expr<int>*, Expr<int>*),
    (Neg,  Expr<int>*)
);

template <>
datatype(Expr<bool>,
    (BoolLit, bool),
    (Not,     Expr<bool>*),
    (And,     Expr<bool>*, Expr<bool>*),
    (IsZero,  Expr<int>*)       // cross-type: int operand, bool result
);

//=============================================================================
// 2.  Evaluators — separate explicit specialisations, each exhaustive
//=============================================================================

template <typename A> A eval(Expr<A>);

template <>
int eval(Expr<int> e) {
    return match(e, {
        of(Num,  n)    { return n; }
        of(Add,  l, r) { return eval(*l) + eval(*r); }
        of(Mul,  l, r) { return eval(*l) * eval(*r); }
        of(Neg,  x)    { return -eval(*x); }
    });
}

template <>
bool eval(Expr<bool> e) {
    return match(e, {
        of(BoolLit, b)    { return b; }
        of(Not,     x)    { return !eval(*x); }
        of(And,     l, r) { return eval(*l) && eval(*r); }
        of(IsZero,  n)    { return eval(*n) == 0; }
    });
}

//=============================================================================
// 3.  Demo: arithmetic expressions
//
//   (2 + 3) * -(4) = -20
//=============================================================================

void demo_arithmetic() {
    Expr<int> n2  = Expr<int>::Num(2);
    Expr<int> n3  = Expr<int>::Num(3);
    Expr<int> n4  = Expr<int>::Num(4);
    Expr<int> add = Expr<int>::Add(&n2, &n3);
    Expr<int> neg = Expr<int>::Neg(&n4);
    Expr<int> mul = Expr<int>::Mul(&add, &neg);

    printf("(2+3)*(-4) = %d\n", eval(mul));   // -20
    assert(eval(mul) == -20);
}

//=============================================================================
// 4.  Demo: boolean expressions with cross-type IsZero
//
//   IsZero(0) && !false  = true
//   IsZero(1) && !false  = false
//=============================================================================

void demo_boolean() {
    Expr<int>  zero  = Expr<int>::Num(0);
    Expr<int>  one   = Expr<int>::Num(1);
    Expr<bool> f     = Expr<bool>::BoolLit(false);
    Expr<bool> not_f = Expr<bool>::Not(&f);
    Expr<bool> iz0   = Expr<bool>::IsZero(&zero);  // true
    Expr<bool> iz1   = Expr<bool>::IsZero(&one);   // false

    Expr<bool> a = Expr<bool>::And(&iz0, &not_f);  // true && true  = true
    Expr<bool> b = Expr<bool>::And(&iz1, &not_f);  // false && true = false

    printf("IsZero(0) && !false = %d\n", (int)eval(a));  // 1
    printf("IsZero(1) && !false = %d\n", (int)eval(b));  // 0
    assert(eval(a) == true);
    assert(eval(b) == false);
}

//=============================================================================
// 5.  Demo: matches and if_let work on specialisations too
//=============================================================================

void demo_predicates() {
    Expr<int> n = Expr<int>::Num(42);
    Expr<int> a = Expr<int>::Add(&n, &n);

    printf("is Num: %d\n",  matches(n, Num));   // 1
    printf("is Add: %d\n",  matches(a, Add));   // 1
    printf("is Mul: %d\n",  matches(a, Mul));   // 0

    if_let(n, Num, v) {
        printf("if_let Num = %d\n", v);          // 42
    }
}

//=============================================================================
// 6.  Demo: deep mixed-type tree
//
//   IsZero(Neg(Mul(2, -2)))  =  IsZero(4)  =  false
//=============================================================================

void demo_deep() {
    Expr<int> n2   = Expr<int>::Num(2);
    Expr<int> nm2  = Expr<int>::Num(-2);
    Expr<int> mul  = Expr<int>::Mul(&n2, &nm2);   // 2 * -2 = -4
    Expr<int> neg  = Expr<int>::Neg(&mul);          // -(-4) = 4
    Expr<bool> iz  = Expr<bool>::IsZero(&neg);      // IsZero(4) = false
    Expr<bool> niz = Expr<bool>::Not(&iz);           // Not(false) = true

    printf("IsZero(-(2*-2)) = %d\n",     (int)eval(iz));   // 0
    printf("Not(IsZero(-(2*-2))) = %d\n",(int)eval(niz));  // 1
    assert(eval(iz)  == false);
    assert(eval(niz) == true);
}

//=============================================================================

int main() {
    puts("=== arithmetic ===");
    demo_arithmetic();

    puts("=== boolean / cross-type ===");
    demo_boolean();

    puts("=== predicates ===");
    demo_predicates();

    puts("=== deep mixed tree ===");
    demo_deep();

    puts("all assertions passed");
}

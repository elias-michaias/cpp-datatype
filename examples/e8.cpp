// clang-format off

// e8.cpp — free constructors and proxy-based polymorphism
//
// Every datatype generates a proxy + free function for each variant.
// The proxy's operator _R() calls _R::VN(args…) for whatever target type _R
// the compiler resolves from context — exactly like Ok/Err/Some/None do for
// Result/Option.  This means:
//
//   • Free functions — no qualification needed:  Circle(5)  not  Shape::Circle(5)
//   • Partial deduction — missing type params filled from context:
//         return Win(v)   fills in E from the function's declared return type
//         return Lose("x") fills in T from the function's declared return type
//
// Sections:
//   1. User-defined two-param result type, don't need to qualify Win/Lose
//   2. Validation pipeline — chaining with free constructors throughout
//   3. Single-param option type — unit-variant proxy fills T from context
//   4. Mixed: functions that return different datatypes using the same call site

#include "../datatype.hpp"
#include <cassert>
#include <cstdio>
#include <cstring>

//=============================================================================
// 1.  User-defined two-param result type
//     Mirrors the built-in Result<T,E> pattern using a custom datatype.
//     The free constructors Win/Lose mirror Ok/Err.
//=============================================================================

template <typename T, typename E>
datatype(Checked,
    (Win,  T),
    (Lose, E)
);

// Win(v)  deduces T from v; E filled from declared return type.
// Lose(e) deduces E from e; T filled from declared return type.
auto parse_positive(int v) -> Checked<int, const char*> {
    if (v > 0) return Win(v);
    return Lose("must be positive");
}

void test() {
    auto x = match(parse_positive(5), {
        of(Win,  v) return v * 2;
        of(Lose, e) return -1;
    });
    if (x > 10) puts("x > 10");
    else        puts("x <= 10");
}


static auto safe_sqrt(int v) -> Checked<int, const char*> {
    if (v < 0) return Lose("negative input");
    // integer square root
    int root = 0;
    while ((root + 1) * (root + 1) <= v) ++root;
    return Win(root * root == v ? root : -1);  // -1 = not a perfect square
}

static void test_two_param_free_ctor() {
    auto show = [](Checked<int, const char*> c) {
        match(c) {
            of(Win,  v) { printf("Win(%d)\n",  v); }
            of(Lose, e) { printf("Lose(%s)\n", e); }
        }
    };
    show(parse_positive(7));    // Win(7)
    show(parse_positive(-3));   // Lose(must be positive)
    show(safe_sqrt(9));         // Win(3)
    show(safe_sqrt(-1));        // Lose(negative input)

    // Static assertions: the proxy returns a Checked, not the proxy struct,
    // once the target type is known via the declared variable type.
    Checked<int, const char*> w = Win(42);
    Checked<int, const char*> l = Lose("oops");
    assert(matches(w, Win)  && w.data.Win._0 == 42);
    assert(matches(l, Lose) && strcmp(l.data.Lose._0, "oops") == 0);

    puts("two-param free ctor: OK");
}

//=============================================================================
// 2.  Validation pipeline
//     parse_int → clamp → to_float, each returning Checked<T, const char*>.
//     Every step uses Win/Lose without qualification, filling in T from the
//     function signature just like the Result example.
//=============================================================================

static auto parse_int(const char* s) -> Checked<int, const char*> {
    if (!s || !s[0]) return Lose("empty");
    int v = 0;
    bool neg = (s[0] == '-');
    for (int i = neg ? 1 : 0; s[i]; ++i) {
        if (s[i] < '0' || s[i] > '9') return Lose("not a number");
        v = v * 10 + (s[i] - '0');
    }
    return Win(neg ? -v : v);
}

static auto clamp(int v, int lo, int hi) -> Checked<int, const char*> {
    if (v < lo) return Lose("below minimum");
    if (v > hi) return Lose("above maximum");
    return Win(v);
}

static auto to_float(int v) -> Checked<float, const char*> {
    return Win(static_cast<float>(v));  // always succeeds here; Win deduces float
}

static void test_pipeline() {
    // Manual chaining: parse → clamp → to_float
    auto process = [](const char* s) -> Checked<float, const char*> {
        auto a = parse_int(s);
        if (!matches(a, Win)) return Lose(a.data.Lose._0);

        auto b = clamp(a.data.Win._0, 0, 100);
        if (!matches(b, Win)) return Lose(b.data.Lose._0);

        return to_float(b.data.Win._0);
    };

    auto show = [](Checked<float, const char*> c) {
        match(c) {
            of(Win,  v) { printf("Win(%.1f)\n", v); }
            of(Lose, e) { printf("Lose(%s)\n",  e); }
        }
    };

    show(process("42"));     // Win(42.0)
    show(process("150"));    // Lose(above maximum)
    show(process("-5"));     // Lose(below minimum)
    show(process("abc"));    // Lose(not a number)
    show(process(""));       // Lose(empty)

    puts("pipeline: OK");
}

//=============================================================================
// 3.  Single-param option type
//     Unit variant has no fields — T is filled entirely from context.
//=============================================================================

template <typename T>
datatype(Maybe,
    (Just, T),
    (Nothing)
);

// Just(v) deduces T from v.  Nothing() fills T from the return type.
static auto find_first_even(int* arr, int n) -> Maybe<int> {
    for (int i = 0; i < n; ++i)
        if (arr[i] % 2 == 0) return Just(arr[i]);
    return Nothing();
}

static void test_single_param_free_ctor() {
    int odds[]  = {1, 3, 5, 7};
    int mixed[] = {1, 3, 4, 7};

    Maybe<int> a = find_first_even(odds,  4);
    Maybe<int> b = find_first_even(mixed, 4);

    assert(matches(a, Nothing));
    assert(matches(b, Just) && b.data.Just._0 == 4);

    match(a) {
        of(Just,    v) { printf("Just(%d)\n", v); }
        of(Nothing)    { puts("Nothing"); }          // Nothing
    }
    match(b) {
        of(Just,    v) { printf("Just(%d)\n", v); }  // Just(4)
        of(Nothing)    { puts("Nothing"); }
    }

    // Lambda context: T filled from lambda's return type annotation
    auto wrap = [](int v) -> Maybe<int> { return Just(v); };
    Maybe<int> c = wrap(99);
    assert(matches(c, Just) && c.data.Just._0 == 99);

    puts("single-param free ctor: OK");
}

//=============================================================================
// 4.  Concrete (non-template) datatype — free functions return exact type
//     No missing type params, but the proxy still works the same way:
//     _R resolves to the concrete type at the assignment site.
//=============================================================================

datatype(Token,
    (TNum,   int),
    (TIdent, const char*),
    (TPlus),
    (TEof)
);

static auto next_token(const char* s, int pos) -> Token {
    if (!s[pos])          return TEof();
    if (s[pos] == '+')    return TPlus();
    if (s[pos] >= '0' && s[pos] <= '9') return TNum(s[pos] - '0');
    return TIdent("id");
}

static void test_concrete_free_ctor() {
    const char* src = "3+5";

    Token t0 = next_token(src, 0);   // TNum(3)
    Token t1 = next_token(src, 1);   // TPlus
    Token t2 = next_token(src, 2);   // TNum(5)
    Token t3 = next_token(src, 3);   // TEof

    assert(matches(t0, TNum)  && t0.data.TNum._0 == 3);
    assert(matches(t1, TPlus));
    assert(matches(t2, TNum)  && t2.data.TNum._0 == 5);
    assert(matches(t3, TEof));

    match(t0) {
        of(TNum,   n) { printf("TNum(%d)\n",  n); }
        of(TIdent, s) { printf("TIdent(%s)\n", s); }
        of(TPlus)     { puts("TPlus"); }
        of(TEof)      { puts("TEof"); }
    }

    puts("concrete free ctor: OK");
}

//=============================================================================

int main() {
    puts("=== two-param free ctor ===");
    test_two_param_free_ctor();

    puts("\n=== validation pipeline ===");
    test_pipeline();

    puts("\n=== single-param free ctor ===");
    test_single_param_free_ctor();

    puts("\n=== concrete free ctor ===");
    test_concrete_free_ctor();
}

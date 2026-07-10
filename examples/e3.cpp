// clang-format off

// e3.cpp — Option<T> and Result<T, E>
//
// Demonstrates:
//   1. Option<T>: construction, match, monadic ops
//   2. Result<T, E>: construction, match, monadic ops
//   3. Using match/match/if_let on the built-in types
//   4. Chaining map / and_then / or_else

#include <cassert>
#include <cstdio>
#include "../datatype.hpp"

// ── helpers ──────────────────────────────────────────────────────────────────

static int  double_it(int x)     { return x * 2; }
static bool is_positive(int x)   { return x > 0; }

// A fallible parse (returns Result)
auto parse_positive(int v) -> Result<int, const char*> {
    if (v > 0) return Ok(v);
    return Err("not positive");
}

// ── Option tests ─────────────────────────────────────────────────────────────

void option_demo() {
    Option<int> s = Some(42);
    Option<int> n = None();

    // statement-form match
    match(s) {
        of(Some, v) { printf("some(%d)\n", v); }   // some(42)
        of(None)    { printf("none\n"); }
    }
    match(n) {
        of(Some, v) { printf("some(%d)\n", v); }
        of(None)    { printf("none\n"); }            // none
    }

    // expression-form match
    int r = match(s, {
        of(Some, v) { return v + 1; }
        of(None)    { return 0;     }
    });
    printf("some+1 = %d\n", r);   // 43

    // monadic map
    auto s2 = s.map(double_it);
    printf("map *2: %d\n", s2.unwrap());   // 84

    auto n2 = n.map(double_it);
    printf("map none is_none: %d\n", n2.is_none());  // 1

    // and_then (flatMap)
    auto s3 = s.and_then([](int v) -> Option<int> {
        return (v > 10) ? Some(v - 10) : None();
    });
    printf("and_then: %d\n", s3.unwrap());   // 32

    // or_else
    auto fallback = n.or_else([] { return Some(99); });
    printf("or_else: %d\n", fallback.unwrap());   // 99

    // filter
    auto filtered = s.filter(is_positive);
    printf("filter pos: %d\n", filtered.unwrap());   // 42
    auto filtered2 = Some(-5).filter(is_positive);
    printf("filter neg is_none: %d\n", filtered2.is_none());  // 1

    // unwrap_or
    printf("unwrap_or: %d\n", n.unwrap_or(-1));   // -1

    // if_let
    if_let(s, Some, v) { printf("if_let some: %d\n", v); }   // 42

    // matches
    assert(matches(s, Some));
    assert(matches(n, None));
    assert(!matches(s, None));
}

// ── Result tests ─────────────────────────────────────────────────────────────

void result_demo() {
    using R = Result<int, const char*>;

    R ok  = Ok(10);
    R err = Err("oops");

    // statement-form match
    match(ok) {
        of(Ok,  v) { printf("Ok(%d)\n",  v); }   // Ok(10)
        of(Err, e) { printf("Err(%s)\n", e); }
    }
    match(err) {
        of(Ok,  v) { printf("Ok(%d)\n",  v); }
        of(Err, e) { printf("Err(%s)\n", e); }   // Err(oops)
    }

    // expression-form match
    const char* msg = match(err, {
        of(Ok,  _) { return "got value"; }
        of(Err, e) { return e;           }
    });
    printf("match err: %s\n", msg);   // oops

    // map
    auto ok2 = ok.map([](int v) { return v * 3; });
    printf("map *3: %d\n", ok2.unwrap());   // 30

    // map_err
    auto err2 = err.map_err([](const char* e) { return e[0]; });
    printf("map_err: %c\n", err2.unwrap_err());   // o

    // and_then chaining
    auto chain = parse_positive(5)
        .and_then([](int v) { return parse_positive(v - 3); })
        .and_then([](int v) { return Ok(v * 10); });
    printf("chain: %d\n", chain.unwrap());   // 20

    auto bad_chain = parse_positive(-1);
    printf("bad chain is_err: %d\n", bad_chain.is_err());  // 1

    // or_else
    auto recovered = err.or_else([](const char*) {
        return Ok(0);
    });
    printf("recovered: %d\n", recovered.unwrap());   // 0

    // unwrap_or
    printf("unwrap_or: %d\n", err.unwrap_or(-1));    // -1

    // if_let
    if_let(ok, Ok, v)   { printf("if_let ok: %d\n", v);   }   // 10
    if_let(err, Err, e) { printf("if_let err: %s\n", e);  }   // oops

    // matches
    assert( matches(ok,  Ok));
    assert( matches(err, Err));
    assert(!matches(ok,  Err));
}

int main() {
    puts("=== Option ===");
    option_demo();
    puts("=== Result ===");
    result_demo();
}

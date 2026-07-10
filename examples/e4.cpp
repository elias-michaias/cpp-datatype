// clang-format off

// e4.cpp — exhaustiveness, nested matching, guards, and match edge cases
//
// Demonstrates:
//   1. Exhaustive match (no otherwise) — -Wswitch fires if a case is missing
//   2. otherwise fallback
//   3. Nested match inside a match arm
//   4. Statement-form match with goto/break to outer loop
//   5. Multi-field variant destructuring
//   6. where(cond) — match guards (3C/D)

#include <cassert>
#include <cstdio>
#include "../datatype.hpp"

// ── Token: a small language token type ───────────────────────────────────────

datatype(Token,
    (TNum,  int),
    (TPlus),
    (TMinus),
    (TMul),
    (TEnd)
);

const char* token_name(Token t) {
    return match(t, {
        of(TNum,   v) { (void)v; return "num"; }
        of(TPlus)     { return "+";   }
        of(TMinus)    { return "-";   }
        of(TMul)      { return "*";   }
        of(TEnd)      { return "end"; }
    });
}

// All 5 variants covered — no otherwise needed, no -Wswitch.
// Removing any arm would produce a compile-time warning.

// ── RGB + Alpha colour ────────────────────────────────────────────────────────

datatype(Colour,
    (Rgb,  int, int, int),
    (Rgba, int, int, int, int),
    (Named, const char*)
);

void print_colour(Colour c) {
    match(c) {
        of(Rgb,   r, g, b)    { printf("rgb(%d,%d,%d)\n", r, g, b); }
        of(Rgba,  r, g, b, a) { printf("rgba(%d,%d,%d,%d)\n", r, g, b, a); }
        of(Named, name)       { printf("named(%s)\n", name); }
    }
}

// ── Nested matching ───────────────────────────────────────────────────────────

template <typename T>
datatype(Maybe,
    (Just, T),
    (Nothing)
);

int safe_double(Maybe<int> m) {
    return match(m, {
        of(Just, v) {
            return match(Maybe<int>::Just(v * 2), {
                of(Just,    x) { return x; }
                of(Nothing)    { return 0; }
            });
        }
        of(Nothing) { return 0; }
    });
}

// ── Outer-loop interaction (statement form) ───────────────────────────────────
//
// Statement form uses a for-once loop — goto/break to enclosing loops work.

void loop_demo() {
    Token tokens[] = {
        Token::TNum(1),
        Token::TPlus(),
        Token::TNum(2),
        Token::TEnd()
    };

    int sum = 0;
    for (int i = 0; i < 4; i++) {
        match(tokens[i]) {
            of(TNum, v) { sum += v; }
            of(TEnd)    { goto done; }   // exit the for-loop
            otherwise   { }
        }
    }
done:
    printf("sum=%d\n", sum);   // 3
}

// ── Multi-field destructuring ────────────────────────────────────────────────

datatype(Vec3,
    (V3, float, float, float)
);

float dot(Vec3 a, Vec3 b) {
    return match(a, {
        of(V3, ax, ay, az) {
            return match(b, {
                of(V3, bx, by, bz) {
                    return ax * bx + ay * by + az * bz;
                }
            });
        }
    });
}

// ── where(cond) — match guards ────────────────────────────────────────────────
//
// where(cond) goes between of(…) and { body }.
// If the condition is false, control jumps to otherwise.
// Requires otherwise to be present in the same match block.
//
// One arm per variant (switch constraint) — multiple guards on the same
// variant belong inside the arm body as a plain if-else chain.

// Guard: only fire when TNum is positive
const char* describe_token(Token t) {
    return match(t, {
        of(TNum, v) where(v > 0) { return "positive num"; }
        otherwise                { return "non-positive or non-num"; }
    });
}

// Guards work in statement form too; goto still works from statement-form match
void print_notable(Token t) {
    match(t) {
        of(TNum, v) where(v == 42) { printf("the answer: %d\n", v); }
        otherwise                  { }
    }
}

// For multi-condition dispatch on the same variant: if-else inside the arm
const char* classify_num(Token t) {
    return match(t, {
        of(TNum, v) {
            if (v > 100) return "large";
            if (v > 0)   return "positive";
            if (v == 0)  return "zero";
            return "negative";
        }
        otherwise { return "not a number"; }
    });
}

// ── main ─────────────────────────────────────────────────────────────────────

int main() {
    // Token names
    Token ts[] = { Token::TNum(7), Token::TPlus(), Token::TMinus(),
                   Token::TMul(),  Token::TEnd() };
    for (auto& t : ts)
        printf("%s ", token_name(t));
    printf("\n");  // num + - * end

    // Colour
    print_colour(Colour::Rgb(255, 128, 0));
    print_colour(Colour::Rgba(0, 0, 255, 200));
    print_colour(Colour::Named("crimson"));

    // Nested match
    printf("safe_double(Just(5)) = %d\n", safe_double(Maybe<int>::Just(5)));    // 10
    printf("safe_double(Nothing) = %d\n", safe_double(Maybe<int>::Nothing()));  // 0

    // Loop + goto (statement form)
    loop_demo();   // sum=3

    // Dot product via multi-field destructuring
    Vec3 v1 = Vec3::V3(1.f, 2.f, 3.f);
    Vec3 v2 = Vec3::V3(4.f, 5.f, 6.f);
    printf("dot = %g\n", dot(v1, v2));   // 32

    // otherwise fallback (expression form)
    Token t = Token::TNum(99);
    const char* kind = match(t, {
        of(TNum, v) { (void)v; return "numeric"; }
        otherwise   { return "other"; }
    });
    printf("kind = %s\n", kind);   // numeric

    // where guards
    Token pos  = Token::TNum(50);
    Token neg  = Token::TNum(-5);
    Token op   = Token::TPlus();
    printf("describe: %s\n", describe_token(pos));  // positive num
    printf("describe: %s\n", describe_token(neg));  // non-positive or non-num
    printf("describe: %s\n", describe_token(op));   // non-positive or non-num

    Token ans = Token::TNum(42);
    print_notable(ans);             // the answer: 42
    print_notable(Token::TNum(7));  // (nothing)

    Token nums2[] = { Token::TNum(200), Token::TNum(50),
                      Token::TNum(0),   Token::TNum(-3) };
    for (auto& tok : nums2)
        printf("classify: %s\n", classify_num(tok));
    // large, positive, zero, negative
}

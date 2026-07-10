// clang-format off

// e5.cpp — real-world patterns: AST evaluation, error propagation, state machine
//
// Demonstrates:
//   1. AST evaluator — recursive datatype + mutual recursion
//   2. Result-based error propagation with and_then chaining
//   3. Finite state machine using datatype for states
//   4. Mixed use of match, match, matches, if_let

#include <cassert>
#include <cstdio>
#include "../datatype.hpp"

//=============================================================================
// 1.  AST evaluator
//=============================================================================

struct Expr;

datatype(Expr,
    (Num,  int),
    (Add,  Expr*, Expr*),
    (Mul,  Expr*, Expr*),
    (Neg,  Expr*)
);

int eval(Expr e) {
    return match(e, {
        of(Num, v)    { return v; }
        of(Add, l, r) { return eval(*l) + eval(*r); }
        of(Mul, l, r) { return eval(*l) * eval(*r); }
        of(Neg, x)    { return -eval(*x); }
    });
}

// (2 + 3) * -(4)  →  (2+3)*(-4)  =  -20
void ast_demo() {
    Expr n2  = Expr::Num(2);
    Expr n3  = Expr::Num(3);
    Expr n4  = Expr::Num(4);
    Expr add = Expr::Add(&n2, &n3);
    Expr neg = Expr::Neg(&n4);
    Expr mul = Expr::Mul(&add, &neg);
    printf("eval = %d\n", eval(mul));   // -20
}

//=============================================================================
// 2.  Result-based error propagation
//=============================================================================

using ParseResult = Result<int, const char*>;

ParseResult parse_int(const char* s) {
    if (!s || s[0] == '\0') return ParseResult::Err("empty input");
    int v = 0;
    bool neg = (s[0] == '-');
    for (int i = neg ? 1 : 0; s[i]; i++) {
        if (s[i] < '0' || s[i] > '9') return ParseResult::Err("not a digit");
        v = v * 10 + (s[i] - '0');
    }
    return ParseResult::Ok(neg ? -v : v);
}

ParseResult safe_div(int a, int b) {
    if (b == 0) return ParseResult::Err("division by zero");
    return ParseResult::Ok(a / b);
}

// Parse two strings, divide, and propagate errors with and_then chaining.
ParseResult parse_and_divide(const char* a, const char* b) {
    return parse_int(a)
        .and_then([b](int x) {
            return parse_int(b)
                .and_then([x](int y) { return safe_div(x, y); });
        });
}

void result_demo() {
    auto show = [](ParseResult r) {
        match(r) {
            of(Ok,  v) { printf("Ok(%d)\n",  v); }
            of(Err, e) { printf("Err(%s)\n", e); }
        }
    };
    show(parse_and_divide("20", "4"));    // Ok(5)
    show(parse_and_divide("10", "0"));    // Err(division by zero)
    show(parse_and_divide("abc", "2"));   // Err(not a digit)
    show(parse_and_divide("",  "2"));     // Err(empty input)
}

//=============================================================================
// 3.  Finite state machine
//=============================================================================

datatype(ConnState,
    (Idle),
    (Connecting, const char*),   // host
    (Connected,  const char*, int), // host, port
    (Error,      const char*)    // message
);

datatype(ConnEvent,
    (Connect,    const char*, int),  // host, port
    (Success),
    (Failure,    const char*),
    (Disconnect)
);

ConnState transition(ConnState state, ConnEvent event) {
    return match(state, {
        of(Idle) {
            if_let(event, Connect, host, port) {
                (void)port;
                return ConnState::Connecting(host);
            }
            return state;
        }
        of(Connecting, host) {
            if (matches(event, Success))
                return match(event, {
                    of(Connect,    _, _) { return ConnState::Idle(); }
                    of(Success)          {
                        return ConnState::Connected(host, 80);
                    }
                    of(Failure,    m)    { return ConnState::Error(m); }
                    of(Disconnect)       { return ConnState::Idle(); }
                });
            if_let(event, Failure, msg) { return ConnState::Error(msg); }
            if_let(event, Disconnect)   { return ConnState::Idle(); }
            return state;
        }
        of(Connected, _, _) {
            if_let(event, Disconnect) { return ConnState::Idle(); }
            return state;
        }
        of(Error, _) {
            return ConnState::Idle();
        }
    });
}

void print_state(ConnState s) {
    match(s) {
        of(Idle)            { puts("Idle"); }
        of(Connecting, h)   { printf("Connecting(%s)\n", h); }
        of(Connected, h, p) { printf("Connected(%s:%d)\n", h, p); }
        of(Error, m)        { printf("Error(%s)\n", m); }
    }
}

void fsm_demo() {
    ConnState s = ConnState::Idle();
    print_state(s);  // Idle

    s = transition(s, ConnEvent::Connect("example.com", 443));
    print_state(s);  // Connecting(example.com)

    s = transition(s, ConnEvent::Success());
    print_state(s);  // Connected(example.com:80)

    s = transition(s, ConnEvent::Disconnect());
    print_state(s);  // Idle

    // Error path
    s = transition(s, ConnEvent::Connect("bad.host", 80));
    s = transition(s, ConnEvent::Failure("timeout"));
    print_state(s);  // Error(timeout)

    s = transition(s, ConnEvent::Connect("recover", 80));
    print_state(s);  // Idle  (error resets on any event)
}

//=============================================================================

int main() {
    puts("=== AST eval ===");
    ast_demo();

    puts("=== Result propagation ===");
    result_demo();

    puts("=== FSM ===");
    fsm_demo();
}

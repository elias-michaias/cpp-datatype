// clang-format off
#ifndef CPP_MATCH_HPP
#define CPP_MATCH_HPP

#include <cassert>
#include <functional>
#include <type_traits>
#include <utility>

//=============================================================================
// §1  Preprocessor infrastructure
//=============================================================================

// Recursive macro expansion — 4^4 = 256 levels of laziness
#define _DT_PARENS      ()
#define _DT_EXPAND(...) _DT_E1(_DT_E1(_DT_E1(_DT_E1(__VA_ARGS__))))
#define _DT_E1(...)     _DT_E2(_DT_E2(_DT_E2(_DT_E2(__VA_ARGS__))))
#define _DT_E2(...)     _DT_E3(_DT_E3(_DT_E3(_DT_E3(__VA_ARGS__))))
#define _DT_E3(...)     _DT_E4(_DT_E4(_DT_E4(_DT_E4(__VA_ARGS__))))
#define _DT_E4(...)     __VA_ARGS__

// Two-level token paste (ensures both operands are expanded first)
#define _DT_CAT(a, b)   _DT_CAT_I(a, b)
#define _DT_CAT_I(a, b) a##b

// Probe/check pair — the backbone of all IS_x detection
#define _DT_PROBE(x)             x, 1
#define _DT_CHECK_N(x, n, ...)   n
#define _DT_CHECK(...)           _DT_CHECK_N(__VA_ARGS__, 0)

// Unwrap a parenthesised token sequence: _DT_UNWRAP((a,b,c)) → a,b,c
#define _DT_UNWRAP_I(...)  __VA_ARGS__
#define _DT_UNWRAP(x)      _DT_UNWRAP_I x

// Count 1–16 non-empty args
#define _DT_VA_COUNT(...) \
    _DT_VA_COUNT_I(__VA_ARGS__, 16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)
#define _DT_VA_COUNT_I( \
    _1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16, N, ...) N

// Count 0–8 args (works when __VA_ARGS__ is empty via __VA_OPT__)
#define _DT_VACOUNT0(...) \
    _DT_VACOUNT0_I(__VA_OPT__(x,) __VA_ARGS__, 8,7,6,5,4,3,2,1,0,0)
#define _DT_VACOUNT0_I(_0,_1,_2,_3,_4,_5,_6,_7,_8, N, ...) N

// FOR_EACH: apply macro(item) to each arg
#define _DT_FOR_EACH(macro, ...) \
    __VA_OPT__(_DT_EXPAND(_DT_FE_H(macro, __VA_ARGS__)))
#define _DT_FE_H(macro, a1, ...) \
    macro(a1) __VA_OPT__(_DT_FE_A _DT_PARENS(macro, __VA_ARGS__))
#define _DT_FE_A() _DT_FE_H

// FOR_EACH_WITH: apply macro(data, item) to each arg
#define _DT_FOR_EACH_WITH(macro, data, ...) \
    __VA_OPT__(_DT_EXPAND(_DT_FEW_H(macro, data, __VA_ARGS__)))
#define _DT_FEW_H(macro, data, a1, ...) \
    macro(data, a1) __VA_OPT__(_DT_FEW_A _DT_PARENS(macro, data, __VA_ARGS__))
#define _DT_FEW_A() _DT_FEW_H

//=============================================================================
// §2  Per-variant codegen building blocks
//=============================================================================

// ── tag enum value ──────────────────────────────────────────────────────────
//   Emits:  VName_tag,
//   Passes _DT_UNWRAP(V) directly so the variant name arrives as a bare token
//   (not as a macro call), making ## safe.
#define _DT_GEN_TAG(_, V)       _DT_GEN_TAG_A(_DT_UNWRAP(V))
#define _DT_GEN_TAG_A(...)      _DT_GEN_TAG_I(__VA_ARGS__)
#define _DT_GEN_TAG_I(VN, ...)  VN##_tag,

// ── inner data struct ───────────────────────────────────────────────────────
//   _DT_DS_1(VN)           → struct _VN { _field_count=0; };
//   _DT_DS_2(VN,T0)        → struct _VN { _field_count=1; T0 _0; };
//   _DT_DS_3(VN,T0,T1)     → …
//   (supports up to 8 payload fields, i.e. _DT_DS_9)
#define _DT_GEN_DS(_, V) \
    _DT_GEN_DS_N(_DT_VA_COUNT(_DT_UNWRAP(V)), _DT_UNWRAP(V))
#define _DT_GEN_DS_N(N, ...) _DT_GEN_DS_N_I(N, __VA_ARGS__)
#define _DT_GEN_DS_N_I(N, ...) _DT_DS_##N(__VA_ARGS__)

#define _DT_DS_1(VN) \
    struct _##VN { static constexpr std::size_t _field_count = 0; };
#define _DT_DS_2(VN, T0) \
    struct _##VN { static constexpr std::size_t _field_count = 1; T0 _0; };
#define _DT_DS_3(VN, T0, T1) \
    struct _##VN { static constexpr std::size_t _field_count = 2; T0 _0; T1 _1; };
#define _DT_DS_4(VN, T0, T1, T2) \
    struct _##VN { static constexpr std::size_t _field_count = 3; T0 _0; T1 _1; T2 _2; };
#define _DT_DS_5(VN, T0, T1, T2, T3) \
    struct _##VN { static constexpr std::size_t _field_count = 4; T0 _0; T1 _1; T2 _2; T3 _3; };
#define _DT_DS_6(VN, T0, T1, T2, T3, T4) \
    struct _##VN { static constexpr std::size_t _field_count = 5; T0 _0; T1 _1; T2 _2; T3 _3; T4 _4; };
#define _DT_DS_7(VN, T0, T1, T2, T3, T4, T5) \
    struct _##VN { static constexpr std::size_t _field_count = 6; T0 _0; T1 _1; T2 _2; T3 _3; T4 _4; T5 _5; };
#define _DT_DS_8(VN, T0, T1, T2, T3, T4, T5, T6) \
    struct _##VN { static constexpr std::size_t _field_count = 7; T0 _0; T1 _1; T2 _2; T3 _3; T4 _4; T5 _5; T6 _6; };
#define _DT_DS_9(VN, T0, T1, T2, T3, T4, T5, T6, T7) \
    struct _##VN { static constexpr std::size_t _field_count = 8; T0 _0; T1 _1; T2 _2; T3 _3; T4 _4; T5 _5; T6 _6; T7 _7; };

// ── union member ────────────────────────────────────────────────────────────
//   Emits:  _VName VName;
//   Same pattern: unwrap first so VName is a bare token before ##.
#define _DT_GEN_UM(_, V)        _DT_GEN_UM_A(_DT_UNWRAP(V))
#define _DT_GEN_UM_A(...)       _DT_GEN_UM_I(__VA_ARGS__)
#define _DT_GEN_UM_I(VN, ...)   _##VN VN;

// ── static value constructor ────────────────────────────────────────────────
//   _DT_CTOR_1(DN, VN)       → static DN VN() { … }
//   _DT_CTOR_2(DN, VN, T0)   → static DN VN(T0 _v0) { … }
//   …
#define _DT_GEN_CTOR(DN, V) \
    _DT_GEN_CTOR_N(DN, _DT_VA_COUNT(_DT_UNWRAP(V)), _DT_UNWRAP(V))
#define _DT_GEN_CTOR_N(DN, N, ...) _DT_GEN_CTOR_N_I(DN, N, __VA_ARGS__)
#define _DT_GEN_CTOR_N_I(DN, N, ...) _DT_CTOR_##N(DN, __VA_ARGS__)

#define _DT_CTOR_1(DN, VN) \
    static DN VN() \
        { DN _r = {}; _r.tag = VN##_tag; return _r; }
#define _DT_CTOR_2(DN, VN, T0) \
    static DN VN(T0 _v0) \
        { DN _r = {}; _r.tag = VN##_tag; _r.data.VN._0 = _v0; return _r; }
#define _DT_CTOR_3(DN, VN, T0, T1) \
    static DN VN(T0 _v0, T1 _v1) \
        { DN _r = {}; _r.tag = VN##_tag; _r.data.VN._0 = _v0; _r.data.VN._1 = _v1; return _r; }
#define _DT_CTOR_4(DN, VN, T0, T1, T2) \
    static DN VN(T0 _v0, T1 _v1, T2 _v2) \
        { DN _r = {}; _r.tag = VN##_tag; \
          _r.data.VN._0 = _v0; _r.data.VN._1 = _v1; _r.data.VN._2 = _v2; return _r; }
#define _DT_CTOR_5(DN, VN, T0, T1, T2, T3) \
    static DN VN(T0 _v0, T1 _v1, T2 _v2, T3 _v3) \
        { DN _r = {}; _r.tag = VN##_tag; \
          _r.data.VN._0 = _v0; _r.data.VN._1 = _v1; \
          _r.data.VN._2 = _v2; _r.data.VN._3 = _v3; return _r; }
#define _DT_CTOR_6(DN, VN, T0, T1, T2, T3, T4) \
    static DN VN(T0 _v0, T1 _v1, T2 _v2, T3 _v3, T4 _v4) \
        { DN _r = {}; _r.tag = VN##_tag; \
          _r.data.VN._0 = _v0; _r.data.VN._1 = _v1; _r.data.VN._2 = _v2; \
          _r.data.VN._3 = _v3; _r.data.VN._4 = _v4; return _r; }
#define _DT_CTOR_7(DN, VN, T0, T1, T2, T3, T4, T5) \
    static DN VN(T0 _v0, T1 _v1, T2 _v2, T3 _v3, T4 _v4, T5 _v5) \
        { DN _r = {}; _r.tag = VN##_tag; \
          _r.data.VN._0 = _v0; _r.data.VN._1 = _v1; _r.data.VN._2 = _v2; \
          _r.data.VN._3 = _v3; _r.data.VN._4 = _v4; _r.data.VN._5 = _v5; return _r; }
#define _DT_CTOR_8(DN, VN, T0, T1, T2, T3, T4, T5, T6) \
    static DN VN(T0 _v0, T1 _v1, T2 _v2, T3 _v3, T4 _v4, T5 _v5, T6 _v6) \
        { DN _r = {}; _r.tag = VN##_tag; \
          _r.data.VN._0 = _v0; _r.data.VN._1 = _v1; _r.data.VN._2 = _v2; \
          _r.data.VN._3 = _v3; _r.data.VN._4 = _v4; _r.data.VN._5 = _v5; \
          _r.data.VN._6 = _v6; return _r; }
#define _DT_CTOR_9(DN, VN, T0, T1, T2, T3, T4, T5, T6, T7) \
    static DN VN(T0 _v0, T1 _v1, T2 _v2, T3 _v3, T4 _v4, T5 _v5, T6 _v6, T7 _v7) \
        { DN _r = {}; _r.tag = VN##_tag; \
          _r.data.VN._0 = _v0; _r.data.VN._1 = _v1; _r.data.VN._2 = _v2; \
          _r.data.VN._3 = _v3; _r.data.VN._4 = _v4; _r.data.VN._5 = _v5; \
          _r.data.VN._6 = _v6; _r.data.VN._7 = _v7; return _r; }

//=============================================================================
// §3  datatype — declare a tagged-union sum type
//=============================================================================
//
// Syntax:
//   datatype(TypeName,
//       (VariantA, FieldType0, FieldType1),   // up to 8 payload types
//       (VariantB),                            // empty (unit) variant
//       ...                                    // up to 16 variants
//   );
//
// Template variant — put `template <…>` on the line immediately above:
//   template <typename T>
//   datatype(Box, (Some, T), (None));
//
//   template <typename T, typename E>
//   datatype(Result, (Ok, T), (Err, E));
//
// Generated API per datatype Name:
//   Name::VariantA_tag, Name::VariantB_tag, …    (enum Tag values)
//   Name::VariantA(f0, f1)                        (static constructors)
//   val.tag                                       (active variant)
//   val.data.VariantA._0, val.data.VariantA._1    (field access)
//   Name::_variant_count                          (constexpr count)
//
// All of: match, of, matches, if_let work on any datatype.
//

// ── Free constructor proxies ─────────────────────────────────────────────────
//
// For every variant, the macro emits a proxy struct + free function OUTSIDE the
// struct body.  The proxy's operator _R() calls _R::VN(args...) for whatever
// target type _R the compiler resolves from context, filling in any type params
// that the variant's fields alone cannot deduce.  This mirrors how Ok/Err/Some/None
// work for Result/Option:
//
//   Shape s   = Circle(5);              // _R = Shape,       E not applicable
//   Box<int> b = Some(42);              // _R = Box<int>,    T deduced from 42
//   Result<int,string> r = Ok(42);      // _R = Result<i,s>, E filled from r's type
//   Box<int> empty = None();            // _R = Box<int>,    T filled from empty
//
// Because the proxy defers type resolution to the assignment site, the same free
// function works for both concrete and template datatypes.
//
// Name-collision note: two datatypes with the same variant name AND arity in the
// same TU will produce a redefinition of DTProxyVN — use the qualified
// DT::VN() form in that case.  The library's own Some/None/Ok/Err are reserved.

#define _DT_PROXY_NAME(VN) _DT_CAT(DTProxy,VN)

#define _DT_GEN_FREE(DN, V) \
    _DT_GEN_FREE_N(_DT_VA_COUNT(_DT_UNWRAP(V)), _DT_UNWRAP(V))
#define _DT_GEN_FREE_N(N,...)  _DT_GEN_FREE_N_I(N,__VA_ARGS__)
#define _DT_GEN_FREE_N_I(N,...) _DT_FREE_##N(__VA_ARGS__)

// Unit variant — 0 fields
#define _DT_FREE_1(VN) \
    struct _DT_PROXY_NAME(VN) { \
        template <typename _R> \
        [[nodiscard]] operator _R() const { return _R::VN(); } \
    }; \
    [[nodiscard]] inline _DT_PROXY_NAME(VN) VN() { return {}; }

// 1 field
#define _DT_FREE_2(VN,T0) \
    template <typename _A0> \
    struct _DT_PROXY_NAME(VN) { \
        _A0 _0; \
        template <typename _R> \
        [[nodiscard]] operator _R() const { return _R::VN(_0); } \
    }; \
    template <typename _A0> \
    [[nodiscard]] inline _DT_PROXY_NAME(VN)<_A0> VN(_A0 _v0) { return {_v0}; }

// 2 fields
#define _DT_FREE_3(VN,T0,T1) \
    template <typename _A0,typename _A1> \
    struct _DT_PROXY_NAME(VN) { \
        _A0 _0; _A1 _1; \
        template <typename _R> \
        [[nodiscard]] operator _R() const { return _R::VN(_0,_1); } \
    }; \
    template <typename _A0,typename _A1> \
    [[nodiscard]] inline _DT_PROXY_NAME(VN)<_A0,_A1> VN(_A0 _v0,_A1 _v1) { return {_v0,_v1}; }

// 3 fields
#define _DT_FREE_4(VN,T0,T1,T2) \
    template <typename _A0,typename _A1,typename _A2> \
    struct _DT_PROXY_NAME(VN) { \
        _A0 _0; _A1 _1; _A2 _2; \
        template <typename _R> \
        [[nodiscard]] operator _R() const { return _R::VN(_0,_1,_2); } \
    }; \
    template <typename _A0,typename _A1,typename _A2> \
    [[nodiscard]] inline _DT_PROXY_NAME(VN)<_A0,_A1,_A2> VN(_A0 _v0,_A1 _v1,_A2 _v2) { return {_v0,_v1,_v2}; }

// 4 fields
#define _DT_FREE_5(VN,T0,T1,T2,T3) \
    template <typename _A0,typename _A1,typename _A2,typename _A3> \
    struct _DT_PROXY_NAME(VN) { \
        _A0 _0; _A1 _1; _A2 _2; _A3 _3; \
        template <typename _R> \
        [[nodiscard]] operator _R() const { return _R::VN(_0,_1,_2,_3); } \
    }; \
    template <typename _A0,typename _A1,typename _A2,typename _A3> \
    [[nodiscard]] inline _DT_PROXY_NAME(VN)<_A0,_A1,_A2,_A3> VN(_A0 _v0,_A1 _v1,_A2 _v2,_A3 _v3) { return {_v0,_v1,_v2,_v3}; }

// 5 fields
#define _DT_FREE_6(VN,T0,T1,T2,T3,T4) \
    template <typename _A0,typename _A1,typename _A2,typename _A3,typename _A4> \
    struct _DT_PROXY_NAME(VN) { \
        _A0 _0; _A1 _1; _A2 _2; _A3 _3; _A4 _4; \
        template <typename _R> \
        [[nodiscard]] operator _R() const { return _R::VN(_0,_1,_2,_3,_4); } \
    }; \
    template <typename _A0,typename _A1,typename _A2,typename _A3,typename _A4> \
    [[nodiscard]] inline _DT_PROXY_NAME(VN)<_A0,_A1,_A2,_A3,_A4> VN(_A0 _v0,_A1 _v1,_A2 _v2,_A3 _v3,_A4 _v4) { return {_v0,_v1,_v2,_v3,_v4}; }

// 6 fields
#define _DT_FREE_7(VN,T0,T1,T2,T3,T4,T5) \
    template <typename _A0,typename _A1,typename _A2,typename _A3,typename _A4,typename _A5> \
    struct _DT_PROXY_NAME(VN) { \
        _A0 _0; _A1 _1; _A2 _2; _A3 _3; _A4 _4; _A5 _5; \
        template <typename _R> \
        [[nodiscard]] operator _R() const { return _R::VN(_0,_1,_2,_3,_4,_5); } \
    }; \
    template <typename _A0,typename _A1,typename _A2,typename _A3,typename _A4,typename _A5> \
    [[nodiscard]] inline _DT_PROXY_NAME(VN)<_A0,_A1,_A2,_A3,_A4,_A5> VN(_A0 _v0,_A1 _v1,_A2 _v2,_A3 _v3,_A4 _v4,_A5 _v5) { return {_v0,_v1,_v2,_v3,_v4,_v5}; }

// 7 fields
#define _DT_FREE_8(VN,T0,T1,T2,T3,T4,T5,T6) \
    template <typename _A0,typename _A1,typename _A2,typename _A3,typename _A4,typename _A5,typename _A6> \
    struct _DT_PROXY_NAME(VN) { \
        _A0 _0; _A1 _1; _A2 _2; _A3 _3; _A4 _4; _A5 _5; _A6 _6; \
        template <typename _R> \
        [[nodiscard]] operator _R() const { return _R::VN(_0,_1,_2,_3,_4,_5,_6); } \
    }; \
    template <typename _A0,typename _A1,typename _A2,typename _A3,typename _A4,typename _A5,typename _A6> \
    [[nodiscard]] inline _DT_PROXY_NAME(VN)<_A0,_A1,_A2,_A3,_A4,_A5,_A6> VN(_A0 _v0,_A1 _v1,_A2 _v2,_A3 _v3,_A4 _v4,_A5 _v5,_A6 _v6) { return {_v0,_v1,_v2,_v3,_v4,_v5,_v6}; }

// 8 fields
#define _DT_FREE_9(VN,T0,T1,T2,T3,T4,T5,T6,T7) \
    template <typename _A0,typename _A1,typename _A2,typename _A3,typename _A4,typename _A5,typename _A6,typename _A7> \
    struct _DT_PROXY_NAME(VN) { \
        _A0 _0; _A1 _1; _A2 _2; _A3 _3; _A4 _4; _A5 _5; _A6 _6; _A7 _7; \
        template <typename _R> \
        [[nodiscard]] operator _R() const { return _R::VN(_0,_1,_2,_3,_4,_5,_6,_7); } \
    }; \
    template <typename _A0,typename _A1,typename _A2,typename _A3,typename _A4,typename _A5,typename _A6,typename _A7> \
    [[nodiscard]] inline _DT_PROXY_NAME(VN)<_A0,_A1,_A2,_A3,_A4,_A5,_A6,_A7> VN(_A0 _v0,_A1 _v1,_A2 _v2,_A3 _v3,_A4 _v4,_A5 _v5,_A6 _v6,_A7 _v7) { return {_v0,_v1,_v2,_v3,_v4,_v5,_v6,_v7}; }

// Detect whether x is a parenthesised token group — the core of inheritance
// dispatch.  If x is (…), the probe macro is "called" and emits _DT_PROBE(~);
// otherwise the probe token just sits there inert and _DT_CHECK returns 0.
#define _DT_IS_TUPLE(x)          _DT_CHECK(_DT_IS_TUPLE_PROBE x)
#define _DT_IS_TUPLE_PROBE(...)  _DT_PROBE(~)

// Get the first variadic argument.
#define _DT_FIRST(x, ...)  x

// datatype body shared by all dispatch paths
#define _DT_DATATYPE_BODY(Name, ...) \
    enum Tag { _DT_FOR_EACH_WITH(_DT_GEN_TAG,  Name, __VA_ARGS__) } tag; \
    _DT_FOR_EACH_WITH(_DT_GEN_DS,   Name, __VA_ARGS__) \
    union { char dummy; _DT_FOR_EACH_WITH(_DT_GEN_UM,   Name, __VA_ARGS__) } data; \
    _DT_FOR_EACH_WITH(_DT_GEN_CTOR, Name, __VA_ARGS__) \
    static constexpr std::size_t _variant_count = _DT_VA_COUNT(__VA_ARGS__);

// No-inheritance path — first arg is already a variant tuple
#define _DT_DATATYPE_1(Name, ...) \
struct Name { \
    _DT_DATATYPE_BODY(Name, __VA_ARGS__) \
}; \
_DT_FOR_EACH_WITH(_DT_GEN_FREE, Name, __VA_ARGS__)

// Inheritance path — collect ALL non-tuple args into a parenthesised base list,
// then emit the struct once the first tuple is reached.
//
// Accumulation uses numbered levels (_DT_ACCUM0 .. _DT_ACCUM7) instead of
// deferred recursion, avoiding any conflict with the _DT_EXPAND calls inside
// _DT_FOR_EACH_WITH (which would be blue-painted if we nested under _DT_EXPAND).
// Supports up to 8 base classes (far more than anyone needs in practice).
//
// Examples:
//   datatype(D, B,          (V))   → struct D : B          { … }
//   datatype(D, B1, B2,     (V))   → struct D : B1, B2     { … }
//   datatype(D, public B,   (V))   → struct D : public B   { … }

// Terminal: all bases are in Acc (parenthesised list), rest are variant tuples
#define _DT_ACCUM_DONE(Name, Acc, ...) \
struct Name : _DT_UNWRAP(Acc) { \
    _DT_DATATYPE_BODY(Name, __VA_ARGS__) \
}; \
_DT_FOR_EACH_WITH(_DT_GEN_FREE, Name, __VA_ARGS__)

// Each level: if next arg F is a tuple → done; else append F to Acc and advance
#define _DT_ACCUM0(Name,Acc,F,...) _DT_CAT(_DT_ACCUM0_,_DT_IS_TUPLE(F))(Name,Acc,F,__VA_ARGS__)
#define _DT_ACCUM0_1 _DT_ACCUM_DONE
#define _DT_ACCUM0_0(Name,Acc,F,...) _DT_ACCUM1(Name,(_DT_UNWRAP(Acc),F),__VA_ARGS__)

#define _DT_ACCUM1(Name,Acc,F,...) _DT_CAT(_DT_ACCUM1_,_DT_IS_TUPLE(F))(Name,Acc,F,__VA_ARGS__)
#define _DT_ACCUM1_1 _DT_ACCUM_DONE
#define _DT_ACCUM1_0(Name,Acc,F,...) _DT_ACCUM2(Name,(_DT_UNWRAP(Acc),F),__VA_ARGS__)

#define _DT_ACCUM2(Name,Acc,F,...) _DT_CAT(_DT_ACCUM2_,_DT_IS_TUPLE(F))(Name,Acc,F,__VA_ARGS__)
#define _DT_ACCUM2_1 _DT_ACCUM_DONE
#define _DT_ACCUM2_0(Name,Acc,F,...) _DT_ACCUM3(Name,(_DT_UNWRAP(Acc),F),__VA_ARGS__)

#define _DT_ACCUM3(Name,Acc,F,...) _DT_CAT(_DT_ACCUM3_,_DT_IS_TUPLE(F))(Name,Acc,F,__VA_ARGS__)
#define _DT_ACCUM3_1 _DT_ACCUM_DONE
#define _DT_ACCUM3_0(Name,Acc,F,...) _DT_ACCUM4(Name,(_DT_UNWRAP(Acc),F),__VA_ARGS__)

#define _DT_ACCUM4(Name,Acc,F,...) _DT_CAT(_DT_ACCUM4_,_DT_IS_TUPLE(F))(Name,Acc,F,__VA_ARGS__)
#define _DT_ACCUM4_1 _DT_ACCUM_DONE
#define _DT_ACCUM4_0(Name,Acc,F,...) _DT_ACCUM5(Name,(_DT_UNWRAP(Acc),F),__VA_ARGS__)

#define _DT_ACCUM5(Name,Acc,F,...) _DT_CAT(_DT_ACCUM5_,_DT_IS_TUPLE(F))(Name,Acc,F,__VA_ARGS__)
#define _DT_ACCUM5_1 _DT_ACCUM_DONE
#define _DT_ACCUM5_0(Name,Acc,F,...) _DT_ACCUM6(Name,(_DT_UNWRAP(Acc),F),__VA_ARGS__)

#define _DT_ACCUM6(Name,Acc,F,...) _DT_CAT(_DT_ACCUM6_,_DT_IS_TUPLE(F))(Name,Acc,F,__VA_ARGS__)
#define _DT_ACCUM6_1 _DT_ACCUM_DONE
#define _DT_ACCUM6_0(Name,Acc,F,...) _DT_ACCUM7(Name,(_DT_UNWRAP(Acc),F),__VA_ARGS__)

#define _DT_ACCUM7(Name,Acc,F,...) _DT_CAT(_DT_ACCUM7_,_DT_IS_TUPLE(F))(Name,Acc,F,__VA_ARGS__)
#define _DT_ACCUM7_1 _DT_ACCUM_DONE
#define _DT_ACCUM7_0(Name,Acc,F,...) _DT_ACCUM_DONE(Name,(_DT_UNWRAP(Acc),F),__VA_ARGS__)

// Entry: seed Acc with the first base and hand off to the accumulation loop
#define _DT_DATATYPE_0(Name, First, ...) _DT_ACCUM0(Name, (First), __VA_ARGS__)

#define datatype(Name, ...) \
    _DT_CAT(_DT_DATATYPE_, _DT_IS_TUPLE(_DT_FIRST(__VA_ARGS__)))(Name, __VA_ARGS__)

//=============================================================================
// §4  match — pattern matching (statement and expression form)
//=============================================================================
//
// Two distinct syntaxes:
//
// Statement form — match(val) { of(…){} … }
//   For side-effecting arms.  val must be an lvalue.
//   goto/break to outer loops work freely (no lambda boundary).
//   match(shape) {
//       of(Circle, r)    { printf("r=%d\n", r); }
//       of(Rect, w, h)   { printf("%dx%d\n", w, h); }
//       of(Point)        { puts("point"); }
//       otherwise        { puts("?"); }         // optional catch-all
//   }
//
// Expression form — match(val, { of(…){ return …; } … })
//   Arms must return a consistent type; the whole expression evaluates to it.
//   val may be an rvalue (lifetime is extended inside the IIFE).
//   auto area = match(shape, {
//       of(Circle, r)  { return r * r; }
//       of(Rect, w, h) { return w * h; }
//       of(Point)      { return 0;     }
//   });
//
// Bindings in of():
//   - fields arrive as T& references — use the name directly, no dereference.
//   - use `_` to ignore a field:  of(Rect, _, h) { use(h); }
//   - pointer fields (e.g. BTree*) bind as T*& ; dereference once to get T.
//
// where(cond) — match guard:
//   of(Circle, r) where(r > 10) { … }
//   If cond is false the arm body is skipped; control jumps to otherwise.
//   Requires otherwise to be present.  Nested statement-form matches
//   that both use where+otherwise share the label _match_otherwise — nesting
//   them in the same function scope is not supported; use expression-form
//   (IIFE) for the inner match instead.
//
// otherwise — catch-all default case (must be last).
//   Also acts as the landing pad for where-guard failures.

// Detect the `_` ignore-binding sentinel
#define _DT_IS_IGN(x)   _DT_CHECK(_DT_CAT(_DT_IS_IGN_, x))
#define _DT_IS_IGN__    _DT_PROBE(~)

// One binding — C++17 if-with-initialiser to scope the reference to the arm body.
// `_` emits a plain `if (true)` so no unused-variable warning.
#define _DT_BIND(name, field) \
    _DT_CAT(_DT_BIND_, _DT_IS_IGN(name))(name, field)
#define _DT_BIND_0(name, field)  if (auto& name = _mdata.field; true)
#define _DT_BIND_1(name, field)  if (true)

// Binding chains (0–8 names)
#define _DT_BINDS_0()                /* nothing — body follows the _mdata if */
#define _DT_BINDS_1(a)               _DT_BIND(a,_0)
#define _DT_BINDS_2(a,b)             _DT_BIND(a,_0) _DT_BIND(b,_1)
#define _DT_BINDS_3(a,b,c)           _DT_BIND(a,_0) _DT_BIND(b,_1) _DT_BIND(c,_2)
#define _DT_BINDS_4(a,b,c,d)         _DT_BIND(a,_0) _DT_BIND(b,_1) _DT_BIND(c,_2) _DT_BIND(d,_3)
#define _DT_BINDS_5(a,b,c,d,e)       _DT_BIND(a,_0) _DT_BIND(b,_1) _DT_BIND(c,_2) _DT_BIND(d,_3) _DT_BIND(e,_4)
#define _DT_BINDS_6(a,b,c,d,e,f)     _DT_BIND(a,_0) _DT_BIND(b,_1) _DT_BIND(c,_2) _DT_BIND(d,_3) _DT_BIND(e,_4) _DT_BIND(f,_5)
#define _DT_BINDS_7(a,b,c,d,e,f,g)   _DT_BIND(a,_0) _DT_BIND(b,_1) _DT_BIND(c,_2) _DT_BIND(d,_3) _DT_BIND(e,_4) _DT_BIND(f,_5) _DT_BIND(g,_6)
#define _DT_BINDS_8(a,b,c,d,e,f,g,h) _DT_BIND(a,_0) _DT_BIND(b,_1) _DT_BIND(c,_2) _DT_BIND(d,_3) _DT_BIND(e,_4) _DT_BIND(f,_5) _DT_BIND(g,_6) _DT_BIND(h,_7)

// Two-level dispatch so N expands before the ## paste.
#define _DT_BINDS_D(N, ...)     _DT_BINDS_D_I(N, __VA_ARGS__)
#define _DT_BINDS_D_I(N, ...)   _DT_BINDS_##N(__VA_ARGS__)

// of(Variant, bindings…)
//
// Expands to:
//   break; case Type::Variant_tag:
//   if ([[maybe_unused]] auto& _mdata = _match_ptr->data.Variant; true)
//   [if (auto& a = _mdata._0; true) …]
//
// The user's { … } (or where(cond) { … }) is the body of the innermost if.
#define of(Variant, ...) \
    break; \
    case std::remove_pointer_t<decltype(_match_ptr)>::Variant##_tag: \
    if ([[maybe_unused]] auto& _mdata = _match_ptr->data.Variant; true) \
    _DT_BINDS_D(_DT_VACOUNT0(__VA_ARGS__) __VA_OPT__(,) __VA_ARGS__)

// where(cond) — guard between of(…) and { body }
// If cond is false, jumps to _match_otherwise (defined by otherwise).
// When cond is true, executes the following { body }.
// Note: only one arm per variant; multiple guards on the same variant
// aren't possible in a switch — use if-else inside the arm instead.
#define where(cond)  if (!(cond)) { goto _match_otherwise; } else

// otherwise — default catch-all arm (must be last).
// Defines _match_otherwise as a goto target for where-guard failures.
#define otherwise  break; _match_otherwise: default:

// Detect whether match() has a second argument (expression form).
#define _DT_MATCH_HAS(...) __VA_OPT__(1)

// Statement form — for-once loop scopes _match_ptr into the following { }.
// val must be an lvalue.  goto/break to enclosing loops work here.
#define _DT_MATCH_(val) \
    for (auto* _match_ptr = &(val); _match_ptr; _match_ptr = nullptr) \
    switch (_match_ptr->tag)

// Expression form — IIFE lambda.  auto&& accepts rvalues and extends their
// lifetime.  Return type deduced from arm return expressions.
#define _DT_MATCH_1(val, body) \
    ([&]() { \
        auto&& _match_val = (val); \
        auto*  _match_ptr = &_match_val; \
        switch (_match_ptr->tag) body \
    }())

// Unified entry point: match(val) → statement form; match(val, {…}) → expression form.
#define match(val, ...) \
    _DT_CAT(_DT_MATCH_, _DT_MATCH_HAS(__VA_ARGS__))(val __VA_OPT__(,) __VA_ARGS__)

//=============================================================================
// §5  Helper macros
//=============================================================================

// matches(val, Variant) → bool
//   True if val currently holds the given variant.
#define matches(val, Variant) \
    ((val).tag == std::remove_reference_t<decltype(val)>::Variant##_tag)

// if_let(val, Variant, bindings…) stmt
//   Executes stmt only when val holds Variant, with T& bindings in scope.
//
//   if_let(opt, Some, v) { printf("%d\n", v); }
//   if_let(res, Err, e)  { printf("err: %s\n", e); }
//   if_let(s,   Point)   { puts("it's a point"); }
#define if_let(val, Variant, ...) \
    if (matches(val, Variant)) \
    if ([[maybe_unused]] auto& _mdata = (val).data.Variant; true) \
    _DT_BINDS_D(_DT_VACOUNT0(__VA_ARGS__) __VA_OPT__(,) __VA_ARGS__)

//=============================================================================
// §7  Option<T>
//=============================================================================
//
// Follows the exact datatype layout — all match macros work on it.
//
// Constructors (free, no template args):
//   Some(42)   →  Option<int>
//   None()     →  Option<T>   (T deduced from context)
//
// Methods:  is_some(), is_none(), unwrap(), unwrap_or(def),
//           map(fn), and_then(fn), or_else(fn), filter(pred)

// Forward declarations so Result's and_then/or_else can specialise on them
template <typename T, typename E> struct Result;
template <typename V> struct _OkProxy;
template <typename E> struct _ErrProxy;

// Trait: given the return type of an and_then fn and the known E, find Ret
template <typename R, typename KT, typename KE> struct _res_and_then { using type = R; };
template <typename V, typename KT, typename KE>
struct _res_and_then<_OkProxy<V>, KT, KE>  { using type = Result<V,  KE>; };
template <typename F, typename KT, typename KE>
struct _res_and_then<_ErrProxy<F>, KT, KE> { using type = Result<KT, F>;  };

// Trait: given the return type of an or_else fn and the known T & E, find Ret
template <typename R, typename KT, typename KE> struct _res_or_else { using type = R; };
template <typename V, typename KT, typename KE>
struct _res_or_else<_OkProxy<V>,  KT, KE> { using type = Result<V,  KE>; };
template <typename F, typename KT, typename KE>
struct _res_or_else<_ErrProxy<F>, KT, KE> { using type = Result<KT, F>;  };

template <typename T>
struct Option {
    enum Tag { Some_tag, None_tag } tag;
    struct _Some { static constexpr std::size_t _field_count = 1; T _0; };
    struct _None { static constexpr std::size_t _field_count = 0; };
    union { char dummy; _Some Some; _None None; } data;
    static constexpr std::size_t _variant_count = 2;

    static Option Some(T v) noexcept {
        Option r = {}; r.tag = Some_tag; r.data.Some._0 = v; return r;
    }
    static Option None() noexcept {
        Option r = {}; r.tag = None_tag; return r;
    }

    [[nodiscard]] bool is_some() const noexcept { return tag == Some_tag; }
    [[nodiscard]] bool is_none() const noexcept { return tag == None_tag; }

    T& unwrap() noexcept {
        assert(is_some() && "Option::unwrap() on None");
        return data.Some._0;
    }
    const T& unwrap() const noexcept {
        assert(is_some() && "Option::unwrap() on None");
        return data.Some._0;
    }

    T unwrap_or(T def) const noexcept {
        return is_some() ? data.Some._0 : def;
    }

    template <typename Fn>
    [[nodiscard]] auto map(Fn&& fn) const
        -> Option<std::__remove_cvref_t<std::invoke_result_t<Fn, const T&>>>
    {
        using U = std::__remove_cvref_t<std::invoke_result_t<Fn, const T&>>;
        if (is_some())
            return Option<U>::Some(std::invoke(std::forward<Fn>(fn), data.Some._0));
        return Option<U>::None();
    }

    template <typename Fn>
    [[nodiscard]] auto and_then(Fn&& fn) const
        -> std::invoke_result_t<Fn, const T&>
    {
        if (is_some()) return std::invoke(std::forward<Fn>(fn), data.Some._0);
        return std::decay_t<std::invoke_result_t<Fn, const T&>>::None();
    }

    template <typename Fn>
    [[nodiscard]] Option or_else(Fn&& fn) const {
        if (is_none()) return std::invoke(std::forward<Fn>(fn));
        return *this;
    }

    template <typename Pred>
    [[nodiscard]] Option filter(Pred&& pred) const {
        if (is_some() && std::invoke(std::forward<Pred>(pred), data.Some._0))
            return *this;
        return None();
    }
};

//=============================================================================
// §8  Result<T, E>
//=============================================================================
//
// Follows the exact datatype layout — all match macros work on it.
//
// Constructors (free, no template args):
//   Ok(42)       →  Result<int, E>         (T deduced, E from context)
//   Err("oops")  →  Result<T, const char*> (E deduced, T from context)
//   Result<int, const char*>::Ok / ::Err still available for disambiguation
//
// Methods:  is_ok(), is_err(), unwrap(), unwrap_err(), unwrap_or(def),
//           map(fn), map_err(fn), and_then(fn), or_else(fn)

template <typename T, typename E>
struct Result {
    enum Tag { Ok_tag, Err_tag } tag;
    struct _Ok  { static constexpr std::size_t _field_count = 1; T _0; };
    struct _Err { static constexpr std::size_t _field_count = 1; E _0; };
    union { char dummy; _Ok Ok; _Err Err; } data;
    static constexpr std::size_t _variant_count = 2;

    static Result Ok(T v) noexcept {
        Result r = {}; r.tag = Ok_tag; r.data.Ok._0 = v; return r;
    }
    static Result Err(E e) noexcept {
        Result r = {}; r.tag = Err_tag; r.data.Err._0 = e; return r;
    }

    [[nodiscard]] bool is_ok()  const noexcept { return tag == Ok_tag;  }
    [[nodiscard]] bool is_err() const noexcept { return tag == Err_tag; }

    T& unwrap() noexcept {
        assert(is_ok() && "Result::unwrap() on Err");
        return data.Ok._0;
    }
    const T& unwrap() const noexcept {
        assert(is_ok() && "Result::unwrap() on Err");
        return data.Ok._0;
    }
    T unwrap_or(T def) const noexcept { return is_ok() ? data.Ok._0 : def; }

    E& unwrap_err() noexcept {
        assert(is_err() && "Result::unwrap_err() on Ok");
        return data.Err._0;
    }
    const E& unwrap_err() const noexcept {
        assert(is_err() && "Result::unwrap_err() on Ok");
        return data.Err._0;
    }

    template <typename Fn>
    [[nodiscard]] auto map(Fn&& fn) const
        -> Result<std::__remove_cvref_t<std::invoke_result_t<Fn, const T&>>, E>
    {
        using U = std::__remove_cvref_t<std::invoke_result_t<Fn, const T&>>;
        if (is_ok())
            return Result<U,E>::Ok(std::invoke(std::forward<Fn>(fn), data.Ok._0));
        return Result<U,E>::Err(data.Err._0);
    }

    template <typename Fn>
    [[nodiscard]] auto map_err(Fn&& fn) const
        -> Result<T, std::__remove_cvref_t<std::invoke_result_t<Fn, const E&>>>
    {
        using F = std::__remove_cvref_t<std::invoke_result_t<Fn, const E&>>;
        if (is_err())
            return Result<T,F>::Err(std::invoke(std::forward<Fn>(fn), data.Err._0));
        return Result<T,F>::Ok(data.Ok._0);
    }

    template <typename Fn>
    [[nodiscard]] auto and_then(Fn&& fn) const {
        using FnRet = std::decay_t<std::invoke_result_t<Fn, const T&>>;
        using Ret   = typename _res_and_then<FnRet, T, E>::type;
        if (is_ok()) { Ret r = std::invoke(std::forward<Fn>(fn), data.Ok._0); return r; }
        return Ret::Err(data.Err._0);
    }

    template <typename Fn>
    [[nodiscard]] auto or_else(Fn&& fn) const {
        using FnRet = std::decay_t<std::invoke_result_t<Fn, const E&>>;
        using Ret   = typename _res_or_else<FnRet, T, E>::type;
        if (is_err()) { Ret r = std::invoke(std::forward<Fn>(fn), data.Err._0); return r; }
        return Ret::Ok(data.Ok._0);
    }
};

//=============================================================================
// §9  Free constructors — type-deducing, no template args required
//=============================================================================
//
//   Some(42)        →  Option<int>            (T deduced from arg)
//   None()          →  Option<T>              (T deduced from context)
//   Ok(42)          →  Result<int, E>         (T deduced from arg, E from context)
//   Err("oops")     →  Result<T, const char*> (E deduced from arg, T from context)
//
// All four work via implicit conversion from a lightweight proxy so the type
// parameters that cannot be deduced from the argument are filled in by the
// compiler from the surrounding context (return type, variable type, etc.).

// ── Some ────────────────────────────────────────────────────────────────────

template <typename V>
[[nodiscard]] Option<std::decay_t<V>> Some(V&& v) noexcept {
    return Option<std::decay_t<V>>::Some(std::forward<V>(v));
}

// ── None ────────────────────────────────────────────────────────────────────

struct _NoneTag {
    template <typename T>
    [[nodiscard]] operator Option<T>() const noexcept { return Option<T>::None(); }
};
[[nodiscard]] inline _NoneTag None() noexcept { return {}; }

// ── Ok ──────────────────────────────────────────────────────────────────────

template <typename V>
struct _OkProxy {
    V val;
    template <typename T, typename E>
    [[nodiscard]] operator Result<T, E>() const noexcept {
        return Result<T, E>::Ok(static_cast<T>(val));
    }
};
template <typename V>
[[nodiscard]] _OkProxy<std::decay_t<V>> Ok(V&& v) noexcept {
    return { std::forward<V>(v) };
}

// ── Err ─────────────────────────────────────────────────────────────────────

template <typename E>
struct _ErrProxy {
    E val;
    template <typename T, typename F>
    [[nodiscard]] operator Result<T, F>() const noexcept {
        return Result<T, F>::Err(static_cast<F>(val));
    }
};
template <typename E>
[[nodiscard]] _ErrProxy<std::decay_t<E>> Err(E&& e) noexcept {
    return { std::forward<E>(e) };
}

//=============================================================================

#endif // CPP_MATCH_HPP

// clang-format off

// e1.cpp — basic datatype declaration and pattern matching
//
// Demonstrates:
//   1. datatype() with unit, single-field, and multi-field variants
//   2. match / of / otherwise — statement and expression use
//   3. _ (ignore) bindings
//   4. matches() predicate
//   5. if_let() conditional binding

#include <cstdio>
#include "../datatype.hpp"

// ── A classic shape sum type ─────────────────────────────────────────────────

datatype(Shape,
    (Circle, int),            // radius
    (Rect,   int, int),       // width, height
    (Point)                   // unit variant — no payload
);

int area(Shape s) {
    return match(s, {
        of(Circle, r) return r * r;
        of(Rect, w, h) return w * h;
        of(Point) return 0;
    });
}

void describe(Shape s) {
    match(s) {
        of(Circle, r) { printf("circle  r=%d  area=%d\n", r, r * r); }
        of(Rect, w, h){ printf("rect    %dx%d  area=%d\n", w, h, w * h); }
        of(Point)     { printf("point\n"); }
    }
}

// ── A recursive binary tree ──────────────────────────────────────────────────

struct BTree;   // forward-declare so the pointer variants compile

datatype(BTree,
    (Leaf, int),
    (Node, BTree*, int, BTree*)
);

int btree_sum(BTree t) {
    return match(t, {
        of(Leaf, v)        { return v; }
        of(Node, l, x, r) { return btree_sum(*l) + x + btree_sum(*r); }
    });
}

// ── main ────────────────────────────────────────────────────────────────────

int main() {
    // Basic shape matching
    Shape c = Shape::Circle(5);
    Shape r = Shape::Rect(3, 4);
    Shape p = Shape::Point();

    describe(c);          // circle  r=5  area=25
    describe(r);          // rect    3x4  area=12
    describe(p);          // point

    printf("area(circle)=%d\n", area(c)); // 25
    printf("area(rect)=%d\n",   area(r)); // 12

    // _ ignore binding
    match(r) {
        of(Rect, _, h) { printf("height=%d\n", h); }   // height=4
        otherwise      { }
    }

    // matches predicate
    printf("is circle: %d\n", matches(c, Circle)); // 1
    printf("is rect:   %d\n", matches(c, Rect));   // 0

    // if_let conditional binding
    if_let(c, Circle, rad) {
        printf("if_let circle rad=%d\n", rad); // if_let circle rad=5
    }
    if_let(c, Rect) {
        printf("should not print\n");
    }

    // Recursive binary tree
    BTree l5  = BTree::Leaf(5);
    BTree l7  = BTree::Leaf(7);
    BTree n   = BTree::Node(&l5, 10, &l7);
    BTree l3  = BTree::Leaf(3);
    BTree big = BTree::Node(&l3, 1, &n);
    printf("tree sum=%d\n", btree_sum(big)); // 3+1+5+10+7 = 26
}

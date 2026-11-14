#include "body_rules.h"

#include <cassert>
#include <cmath>
#include <iostream>

using namespace BodyRules;

namespace {

constexpr float kEpsilon = 1e-4f;

bool Near(float a, float b) {
    return std::fabs(a - b) <= kEpsilon;
}

bool Contains(const Rectangle& r, Vector2 p) {
    return p.x >= r.x && p.x <= r.x + r.width && p.y >= r.y && p.y <= r.y + r.height;
}

void TestHasBody() {
    assert(!HasBody(BodyGeometry{}));
    assert(!HasBody(BodyGeometry{40.0f, 0.0f, 10.0f}));
    assert(!HasBody(BodyGeometry{0.0f, 80.0f, 10.0f}));
    assert(HasBody(BodyGeometry{40.0f, 80.0f, 30.0f}));
}

void TestHurtboxRestsOnFeetAndIsCentered() {
    const BodyGeometry body{40.0f, 80.0f, 30.0f};
    const Vector2 feet{100.0f, 200.0f};
    Rectangle box = Hurtbox(feet, body);
    assert(Near(box.width, 40.0f));
    assert(Near(box.height, 80.0f));
    assert(Near(box.y + box.height, feet.y));
    assert(Near(box.x + box.width * 0.5f, feet.x));
}

void TestHandAnchorIsAboveFeetInsideHurtbox() {
    const BodyGeometry body{40.0f, 80.0f, 30.0f};
    const Vector2 feet{-50.0f, 10.0f};
    Vector2 hand = HandAnchor(feet, body);
    assert(Near(hand.x, feet.x));
    assert(Near(hand.y, feet.y - 30.0f));
    assert(Contains(Hurtbox(feet, body), hand));
}

void TestCenterIsMiddleOfHurtbox() {
    const BodyGeometry body{36.0f, 84.0f, 36.0f};
    const Vector2 feet{12.0f, 34.0f};
    Rectangle box = Hurtbox(feet, body);
    Vector2 center = Center(feet, body);
    assert(Near(center.x, box.x + box.width * 0.5f));
    assert(Near(center.y, box.y + box.height * 0.5f));
}

}  // namespace

int main() {
    TestHasBody();
    TestHurtboxRestsOnFeetAndIsCentered();
    TestHandAnchorIsAboveFeetInsideHurtbox();
    TestCenterIsMiddleOfHurtbox();
    std::cout << "Body rules tests passed." << std::endl;
    return 0;
}

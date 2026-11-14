#include "body_rules.h"

namespace BodyRules {

bool HasBody(const BodyGeometry& body) {
    return body.width > 0.0f && body.height > 0.0f;
}

Rectangle Hurtbox(Vector2 feet, const BodyGeometry& body) {
    return Rectangle{feet.x - body.width * 0.5f, feet.y - body.height, body.width, body.height};
}

Vector2 HandAnchor(Vector2 feet, const BodyGeometry& body) {
    return Vector2{feet.x, feet.y - body.handHeight};
}

Vector2 Center(Vector2 feet, const BodyGeometry& body) {
    return Vector2{feet.x, feet.y - body.height * 0.5f};
}

}  // namespace BodyRules

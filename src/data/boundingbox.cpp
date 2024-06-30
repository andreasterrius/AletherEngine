//
// Created by Alether on 4/17/2024.
//

#include "boundingbox.h"
#include "transform.h"

ale::BoundingBox::BoundingBox(vec3 min, vec3 max) : min(min), max(max) {
    this->center = vec3(
        min.x + (max.x - min.x) / 2,
        min.y + (max.y - min.y) / 2,
        min.z + (max.z - min.z) / 2
    );
}

ale::BoundingBox ale::BoundingBox::applyTransform(ale::Transform t) const {
    return BoundingBox(
        t.getModelMatrix() * vec4(this->min, 1.0),
        t.getModelMatrix() * vec4(this->max, 1.0)
    );
}

vec3 ale::BoundingBox::getCenter() const {
    return this->center;
}

vec3 ale::BoundingBox::getSize() const {
    return this->max - this->min;
}


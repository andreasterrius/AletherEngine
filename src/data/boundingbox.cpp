//
// Created by Alether on 4/17/2024.
//

#include "boundingbox.h"
#include "transform.h"

ale::BoundingBox::BoundingBox(vec3 min, vec3 max) : min(min), max(max) {

}

ale::BoundingBox ale::BoundingBox::applyTransform(ale::Transform t) const {
    return BoundingBox(
            t.getModelMatrix() * vec4(this->min, 1.0),
            t.getModelMatrix() * vec4(this->max, 1.0)
        );
}

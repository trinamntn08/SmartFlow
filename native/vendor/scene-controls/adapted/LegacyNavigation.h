#pragma once
// Adapted from tp_maps CADController.cpp, MIT, Copyright (c) 2018 Thomas David Paynter.
// See ../tp_maps/LICENSE and ../copy-manifest.json. SmartFlow uses a Y-up basis.
#include <glm/glm.hpp>

namespace smartflow::legacy {
inline glm::vec2 orthographicPan(float dx, float dy, float width, float height, float span)
{
    if(width <= 0 || height <= 0) return {};
    const float fw = width > height ? width / height : 1.0f;
    const float fh = height > width ? height / width : 1.0f;
    // Legacy distance is half the shorter viewport span.
    return {dx / width * fw * span, dy / height * fh * span};
}
}

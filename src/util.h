//
// Created by Alether on 4/16/2024.
//

#ifndef ALETHERENGINE_UTIL_H
#define ALETHERENGINE_UTIL_H

#include <glad/glad.h>
#include <stb_image.h>
#include <optional>

class Util {
public:
    static std::optional<unsigned int> loadTexture(char const * path);
};


#endif //ALETHERENGINE_UTIL_H

#ifndef ALETHERENGINE_SHADER_H
#define ALETHERENGINE_SHADER_H

#include <string>
#include <vector>

import data;

struct IncludeDirective {
  std::string filename;
  std::vector<int> bindings;
};

std::string remove_include_lines(const std::string &input,
                                 bool replace_with_spaces);

std::string load_file_with_include(IncludeDirective include_directive);

#endif

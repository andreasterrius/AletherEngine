#include "shader.h"

#include <regex>

#include "../file_system.h"

using afs = ale::FileSystem;

std::string Shader::load_file_with_include(IncludeDirective include_directive) {
  using namespace std;

  string path = include_directive.filename;
  if (path == "") {
    throw runtime_error("no path is provided");
  }

  auto shader_file = ifstream{};  // open files
  string code;
  {
    shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    shader_file.open(path);
    if (!shader_file.is_open()) {
      throw runtime_error("failed to open file: " + path);
    }

    stringstream shader_sstream;
    shader_sstream << shader_file.rdbuf();
    code = shader_sstream.str();
    shader_file.close();
  }

  // replace all according to include directives
  {
    size_t pos = 0;
    for (size_t i = 0; i < include_directive.bindings.size(); ++i) {
      // Form the placeholder string, e.g., "<0>", "<1>", ...
      std::string placeholder = "<" + std::to_string(i) + ">";
      // Find the position of the placeholder
      while ((pos = code.find(placeholder)) != std::string::npos) {
        // Replace the placeholder with the corresponding value
        code.replace(pos, placeholder.length(),
                     std::to_string(include_directive.bindings[i]));
      }
    }
  }

  string new_code;
  {
    regex pattern("#include\\s+\"([^\"]+)\"\\s+([\\d\\s]+)");
    std::string::const_iterator prev_pos = code.begin();
    auto begin = sregex_iterator(code.begin(), code.end(), pattern);
    auto end = sregex_iterator();
    // get regex groups
    for (auto it = begin; it != end; ++it) {
      auto include_directive = IncludeDirective{};
      auto match = *it;
      if (match.size() < 2) {
        throw runtime_error("unable to parse include directive for: " + path);
      }
      include_directive.filename = afs::root(match[1]);

      if (match.size() == 3) {
        stringstream binding_sstr(match[2]);

        int number;
        while (binding_sstr >> number) {
          include_directive.bindings.push_back(number);
        }
      }

      new_code.append(prev_pos, match[0].first);
      string included_code = this->load_file_with_include(include_directive);
      new_code.append(included_code);
      prev_pos = match[0].second;
    }
    std::string::const_iterator cend = code.end();
    new_code.append(prev_pos, cend);
  }

  return new_code;
}

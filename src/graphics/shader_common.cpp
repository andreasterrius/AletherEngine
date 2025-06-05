
#include "shader_common.h"
#include <fstream>
#include <iterator>
#include <regex>
#include <sstream>

// namespace ale::graphics
std::string remove_include_lines(const std::string &input,
                                 bool replace_with_spaces) {
  // Split input into lines
  std::regex line_re(R"(([^\n]*)(\n?))");
  std::sregex_iterator start(input.begin(), input.end(), line_re);
  std::sregex_iterator end;
  std::string result;

  // Regex to identify #include lines (with optional leading/trailing
  // whitespace)
  std::regex include_re(R"(\s*#include\s*["<][^">]*[">]\s*)");
  for (auto it = start; it != end; ++it) {
    std::smatch match = *it;
    std::string line_content = match[1]; // Line text (without newline)
    std::string line_ending = match[2]; // Newline character(s)
    if (std::regex_match(line_content, include_re)) {
      if (replace_with_spaces) {
        // Replace line content with spaces, preserve line ending
        result += std::string(line_content.size(), ' ') + line_ending;
      } else {
        // Remove the line entirely (only keep line ending if not part of the
        // match)
        result += line_ending;
      }
    } else {
      // Keep non-#include lines unchanged
      result += line_content + line_ending;
    }
  }

  return result;
}

std::string load_file_with_include(IncludeDirective include_directive) {
  using namespace std;
  using namespace std::regex_constants;

  string path = include_directive.filename;
  if (path == "") {
    throw runtime_error("no path is provided");
  }

  auto shader_file = ifstream{}; // open files
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

  // special case, we stop including anything if it contains this string
  if (include_directive.filename.find("_shared.cpp") != std::string::npos) {
    code = remove_include_lines(code, false);
    return code;
  }

  string new_code;
  {
    std::regex pattern("#include\\s+\"([^\"]+)\"\\s+([\\d\\s]+)");
    std::string::const_iterator prev_pos = code.begin();
    std::sregex_iterator begin(code.begin(), code.end(), pattern,
                               regex_constants::_Skip_zero_length);
    std::sregex_iterator end;
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
      string included_code = load_file_with_include(include_directive);
      new_code.append(included_code);
      prev_pos = match[0].second;
    }
    std::string::const_iterator cend = code.end();
    new_code.append(prev_pos, cend);
  }

  return new_code;
}

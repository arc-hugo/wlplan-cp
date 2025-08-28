#include "../../include/planning/action_schema.hpp"

#include <stdexcept>
#include <algorithm>
#include <cctype>
#include <string>

namespace planning {
  ActionSchema::ActionSchema(const std::string &name, const int arity) : name(name), arity(arity) {
    if (arity < 0) {
      std::string err_msg = "ActionSchema " + name + " has arity " + std::to_string(arity) + " < 0";
      throw std::invalid_argument(err_msg);
    }

    std::transform(this->name.begin(), this->name.end(), this->name.begin(), [](unsigned char c) {
      return std::tolower(c);
    });
  }

  std::string ActionSchema::to_string() const { return name + "/" + std::to_string(arity); }
}  // namespace planning

#include "../../include/planning/action_schema.hpp"

#include <stdexcept>

namespace planning {
  ActionSchema::ActionSchema(const std::string &name, const int arity) : name(name), arity(arity) {
    if (arity < 0) {
      std::string err_msg = "ActionSchema " + name + " has arity " + std::to_string(arity) + " < 0";
      throw std::invalid_argument(err_msg);
    }
  }

  std::string ActionSchema::to_string() const { return name + "/" + std::to_string(arity); }
}  // namespace planning

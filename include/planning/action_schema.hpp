#ifndef PLANNING_ACTION_SCHEMA_HPP
#define PLANNING_ACTION_SCHEMA_HPP

#include <string>

namespace planning {
  class ActionSchema {
   public:
    std::string name;
    int arity;

    ActionSchema() = default;

    ActionSchema(const std::string &name, const int arity);

    std::string to_string() const;

    bool operator==(const ActionSchema &other) const { return to_string() == other.to_string(); }

    bool operator<(const ActionSchema &other) const { return to_string() < other.to_string(); }
  };

}  // namespace planning

template <> class std::hash<planning::ActionSchema> {
  std::size_t operator()(const planning::ActionSchema &k) const {
    // Compute individual hash values for first,
    // second and third and combine them using XOR
    // and bit shifting:

    return (std::hash<std::string>()(k.name) ^ (std::hash<int>()(k.arity) << 1)) >> 1;
  }
};

#endif  // PLANNING_ACTION_SCHEMA_HPP

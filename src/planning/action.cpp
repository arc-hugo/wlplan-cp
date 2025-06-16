#include "../../include/planning/action.hpp"

#include <stdexcept>

namespace planning {
  Action::Action(const planning::ActionSchema &action_schema,
           const std::set<std::pair<int, int>> &preconditions,
           const std::set<std::pair<int, int>> &effects) : 
           action_schema(action_schema),
           preconditions(preconditions),
           effects(effects) {}

  std::string Action::to_string() const { 
    std::string ret = action_schema.to_string() + "(precond=[";
    size_t i = 0;
    for (auto precond : preconditions) {
      ret += std::to_string(precond.first) + "<-" + std::to_string(precond.second);

      if (i < (preconditions.size() - 1)) {
        ret += ", ";
      }
      i++;
    }

    ret += "], effects=[";

    i = 0;
    for (auto eff : effects) {
      ret += std::to_string(eff.first) + "->" + std::to_string(eff.second);

      if (i < (effects.size() - 1)) {
        ret += ", ";
      }
      i++;
    }

    ret += "])";
    return ret;
  }

  std::set<int> get_indexes(const std::set<std::pair<int, int>> pair_set) {
    std::set<int> ret;
    for (auto e : pair_set) {
      ret.insert(e.first);
    }
    return ret;
  }

  std::set<int> Action::get_preconditions_indexes() const { return get_indexes(preconditions); }
  std::set<int> Action::get_effects_indexes() const { return get_indexes(effects); }

  std::size_t Action::hash() const { return std::hash<std::string>()(this->to_string()); }
}  // namespace planning

#ifndef PLANNING_ACTION_HPP
#define PLANNING_ACTION_HPP

#include "variable.hpp"
#include "action_schema.hpp"

#include <memory>
#include <vector>
#include <set>

namespace planning {
  class Action {
   public:
    planning::ActionSchema action_schema;

    std::set<std::pair<int, int>> preconditions;
    std::set<std::pair<int, int>> effects;

    Action() = default;

    Action(const planning::ActionSchema &action_schema,
           const std::set<std::pair<int, int>> &preconditions,
           const std::set<std::pair<int, int>> &effects);

    std::string to_string() const;

    planning::ActionSchema get_action_shema() const { return action_schema; };

    std::set<std::pair<int,int>> get_preconditions() const { return preconditions; };
    std::set<std::pair<int,int>> get_effects() const  { return effects; };

    std::set<int> get_preconditions_indexes() const;
    std::set<int> get_effects_indexes() const;

    bool operator==(const Action &other) const { return to_string() == other.to_string(); }

    bool operator<(const Action &other) const { return to_string() < other.to_string(); }

    std::size_t hash() const;
  };

}  // namespace planning

#endif  // PLANNING_ACTION_HPP
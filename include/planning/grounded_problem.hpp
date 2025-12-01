#ifndef PLANNING_GROUNDED_PROBLEM_HPP
#define PLANNING_GROUNDED_PROBLEM_HPP

#include "domain.hpp"
#include "action.hpp"
#include "variable.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace planning {
  class GroundedProblem {
   private:
    std::shared_ptr<Domain> domain;

    std::vector<std::string> variable_names;
    std::vector<std::vector<std::string>> variable_values_names;

    std::vector<std::tuple<int, int>> goals;
    std::unordered_map<int, int> goals_map;

    std::vector<planning::Action> actions;

   public:
    GroundedProblem(const Domain &domain,
            const std::vector<std::string> &variable_names,
            const std::vector<std::vector<std::string>> &variable_values_names,
            const std::vector<std::tuple<int, int>> &goals,
            const std::vector<planning::Action> &actions);

    Domain get_domain() const { return *domain; }

    const std::vector<std::string> get_variable_names() const { return variable_names; };
    const std::vector<std::vector<std::string>> get_variable_values_names() const { return variable_values_names; };

    const std::vector<std::tuple<int, int>> get_goals() const { return goals; };
    const std::unordered_map<int, int> get_goals_map() const { return goals_map; };

    const std::vector<planning::Action> get_actions() const { return actions; };

    void dump() const;
  };

}  // namespace planning

#endif  // PLANNING_GROUNDED_PROBLEM_HPP

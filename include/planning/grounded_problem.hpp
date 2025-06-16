#ifndef PLANNING_GROUNDED_PROBLEM_HPP
#define PLANNING_GROUNDED_PROBLEM_HPP

#include "domain.hpp"
#include "action.hpp"
#include "variable.hpp"

#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace planning {
  class GroundedProblem {
   private:
    std::shared_ptr<Domain> domain;

    std::vector<std::string> variable_names;
    std::vector<int> variable_domain_sizes;

    std::vector<std::tuple<int, int>> goals;

    std::vector<planning::Action> actions;

   public:
    GroundedProblem(const Domain &domain,
            const std::vector<std::string> &variable_names,
            const std::vector<int> &variable_domain_sizes,
            const std::vector<std::tuple<int, int>> &goals,
            const std::vector<planning::Action> &actions);

    Domain get_domain() const { return *domain; }

    std::vector<std::string> get_variable_names() const { return variable_names; };
    std::vector<int> get_variable_domain_sizes() const { return variable_domain_sizes; };

    std::vector<std::tuple<int, int>> get_goals() const { return goals; };

    std::vector<planning::Action> get_actions() const { return actions; };

    void dump() const;
  };

}  // namespace planning

#endif  // PLANNING_GROUNDED_PROBLEM_HPP

#include "../../include/planning/grounded_problem.hpp"

#include <algorithm>
#include <iostream>

namespace planning {
  GroundedProblem::GroundedProblem(const Domain &domain,
                  const std::vector<std::string> &variable_names,
                  const std::vector<int> &variable_domain_sizes,
                  const std::vector<std::tuple<int, int>> &goals,
                  const std::vector<planning::Action> &actions)
      : domain(std::make_shared<Domain>(domain)),
        variable_names(variable_names),
        variable_domain_sizes(variable_domain_sizes),
        goals(goals),
        actions(actions) {

    // variables
    if (variable_names.size() != variable_domain_sizes.size()) {
      std::cout << "Error: Number of variables and domains do not match." << std::endl;
      std::exit(1);
    }

    // goals
    if (variable_names.size() < goals.size()) {
      std::cout << "Error: More goals than variables." << std::endl;
      std::exit(1);
    }
  }

  void GroundedProblem::dump() const {
    std::cout << "domain=" << domain->to_string() << std::endl;
    std::cout << "variables=[" << std::endl;
    for (size_t i = 0; i < variable_names.size(); i++)
    {
      std::cout << variable_names[i] << "/" << std::to_string(variable_domain_sizes[i]);
      if (i < (variable_domain_sizes.size() - 1)) {
        std::cout << ", ";
      }
    }
    std::cout << "]" << std::endl;
    std::cout << "goals=[" << std::endl;
    for (size_t i = 0; i < goals.size(); i++)
    {
      std::cout << "(" << variable_names[std::get<0>(goals[i])] << ", " << std::to_string(std::get<1>(goals[i])) << ")";
      if (i < (goals.size() - 1)) {
        std::cout << ", ";
      }
    }

    std::cout << "]" << std::endl;
    std::cout << "actions=[" << std::endl;
    for (size_t i = 0; i < actions.size(); i++)
    {
      std::cout << actions[i].to_string();
      if (i < (actions.size() - 1)) {
        std::cout << ", ";
      }
    }

    std::cout << "]" << std::endl;
  }
}  // namespace planning

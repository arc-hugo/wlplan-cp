#include "../../include/planning/grounded_state.hpp"

#include <algorithm>

namespace planning
{
    void GroundedState::sort_variables() {
      std::sort(
          variables.begin(),
          variables.end(),
          [](std::shared_ptr<planning::Variable> const &v1,
             std::shared_ptr<planning::Variable> const &v2) { 
              return v1->index < v2->index; 
      });
    }

    GroundedState::GroundedState(
        const std::vector<std::shared_ptr<planning::Variable>> &variables) :
        variables(variables) {
      sort_variables();
    }

    GroundedState::GroundedState(
    const std::vector<planning::Variable> &variables) {
      for (const planning::Variable &var : variables)
      {
        this->variables.push_back(std::make_shared<planning::Variable>(var));
      }
      sort_variables();
    }

    std::vector<planning::Variable> GroundedState::get_variables() const {
        std::vector<planning::Variable> ret;
        for (const std::shared_ptr<planning::Variable> &var : variables) {
            ret.push_back(*var);
        }
        return ret;
    }

    std::string GroundedState::to_string() const {
    std::string ret = "GroundedState([";

    // sort atoms because order does not matter
    std::vector<std::tuple<int,std::string,int>> variable_strings;
    for (size_t i = 0; i < variables.size(); i++) {
      variable_strings.push_back(std::make_tuple(variables[i]->index, variables[i]->name, variables[i]->value));
    }
    std::sort(variable_strings.begin(), variable_strings.end(), [](auto const &t1, auto const &t2) {
      return std::get<0>(t1) < std::get<0>(t2);
    });

    for (size_t i = 0; i < variable_strings.size(); i++) {
      ret += std::get<1>(variable_strings[i]) + ":" + std::to_string(std::get<2>(variable_strings[i]));
      if (i < variable_strings.size() - 1) {
        ret += ", ";
      }
    }

    ret += "])";
    
    return ret;
  }


  bool GroundedState::operator==(const GroundedState &other) const {
    return variables == other.variables;
  }

  size_t GroundedState::hash() const { return std::hash<std::string>()(to_string()); }
    
    
} // namespace planning

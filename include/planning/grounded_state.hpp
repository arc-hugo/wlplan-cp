#ifndef PLANNING_GROUNDED_STATE_HPP
#define PLANNING_GROUNDED_STATE_HPP

#include "variable.hpp"

#include <memory>
#include <vector>

namespace planning {
  class GroundedState {
    public:
    std::vector<std::shared_ptr<planning::Variable>> variables;

    GroundedState(const std::vector<std::shared_ptr<planning::Variable>> &variables);
    GroundedState(const std::vector<planning::Variable> &variables);

    // for Python bindings
    std::vector<planning::Variable> get_variables() const;

    std::string to_string() const;

    bool operator==(const GroundedState &other) const;

    std::size_t hash() const;

    private:
    void sort_variables();
  };
}  // namespace planning

#endif  // PLANNING_STATE_HPP

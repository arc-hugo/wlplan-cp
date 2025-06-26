#ifndef PLANNING_VARIABLE_HPP
#define PLANNING_VARIABLE_HPP

#include <string>

namespace planning
{
  class Variable {
    public:
     const std::string name;
     const int index;
     const int value;

     Variable(std::string name, int index, int value) : name(name), index(index), value(value) {}
     Variable(std::string name, int index) : Variable(name, index, -1) {}

     bool operator==(const Variable &other) const { return name == other.name && index == other.index && value == other.value; }
  };
}  // namespace planning

#endif
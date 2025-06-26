#include "../../include/data/dataset.hpp"

#include <iostream>

namespace data {
  Dataset::Dataset(const planning::Domain &domain) 
      : domain(domain) {}

  LiftedDataset::LiftedDataset(const planning::Domain &domain, const std::vector<ProblemStates> &data)
      : Dataset(domain), data(data) {
    
    for (const auto &predicate : domain.predicates) {
      predicate_to_arity[predicate.name] = predicate.arity;
    }

    for (size_t i = 0; i < data.size(); i++) {
      const auto &problem_states = data[i];
      const planning::Problem &problem = problem_states.problem;
      const auto &states = problem_states.states;

      // check domain consistency
      if (!(problem.get_domain() == domain)) {
        std::string err_msg =
            "Domain mismatch between domain and problem in data[" + std::to_string(i) + "]";
        throw std::runtime_error(err_msg);
      }

      std::unordered_set<planning::Object> objects;
      for (const planning::Object &object : problem.get_problem_objects()) {
        objects.insert(object);
      }
      for (const planning::Object &object : problem.get_constant_objects()) {
        objects.insert(object);
      }

      // check proposition consistency of goals
      for (const planning::Atom &goal : problem.get_positive_goals()) {
        check_good_atom(goal, objects);
      }

      for (const planning::Atom &goal : problem.get_negative_goals()) {
        check_good_atom(goal, objects);
      }

      // check proposition consistency of states
      for (const planning::State &state : states) {
        for (const std::shared_ptr<planning::Atom> &atom : state.atoms) {
          check_good_atom(*atom, objects);
        }
      }
    }
  }

  void LiftedDataset::check_good_atom(const planning::Atom &atom,
                                const std::unordered_set<planning::Object> &objects) const {
    if (predicate_to_arity.find(atom.predicate->name) == predicate_to_arity.end()) {
      throw std::runtime_error("Unknown predicate " + atom.predicate->name);
    }

    if (predicate_to_arity.at(atom.predicate->name) != (int)atom.objects.size()) {
      throw std::runtime_error("Arity mismatch for " + atom.to_string());
    }

    for (const planning::Object &object : atom.objects) {
      if (objects.count(object) == 0) {
        throw std::runtime_error("Unknown object " + object);
      }
    }
  }

  size_t LiftedDataset::get_size() const {
    size_t ret = 0;
    for (const auto &problem_states : data) {
      ret += problem_states.states.size();
    }
    return ret;
  }

  std::vector<graph::Graph> LiftedDataset::get_graphs(std::shared_ptr<graph::GraphGenerator> graph_generator) const {
    std::vector<graph::Graph> graphs;
    std::cout << this << std::endl;

    for (size_t i = 0; i < data.size(); i++) {
      const auto &problem_states = data[i];
      const auto &problem = problem_states.problem;
      const auto &states = problem_states.states;
      graph_generator->set_problem(problem);
      for (const planning::State &state : states) {
        graphs.push_back(*(graph_generator->to_graph(state)));          
      }
    }

    return graphs;
  }

  GroundedDataset::GroundedDataset(const planning::Domain &domain,
                                   const std::vector<ProblemPatternsAssignments> &data)
      : Dataset(domain), data(data) {

    for (size_t i = 0; i < data.size(); i++) {
      const auto &problem_states = data[i];
      const auto &problem = problem_states.problem;

      // check domain consistency
      if (!(problem.get_domain() == domain)) {
        std::string err_msg =
            "Domain mismatch between domain and problem in data[" + std::to_string(i) + "]";
        throw std::runtime_error(err_msg);
      }
    }
  }

  size_t GroundedDataset::get_size() const {
    size_t ret = 0;
    for (const auto &problem_states : data) {
      ret += problem_states.assignments.size();
    }
    return ret;
  }


  std::vector<graph::Graph> GroundedDataset::get_graphs(std::shared_ptr<graph::GraphGenerator> graph_generator) const {
    std::vector<graph::Graph> graphs;

    for (size_t i = 0; i < data.size(); i++) {
      const auto &problem_states = data[i];
      const auto &problem = problem_states.problem;
      const auto &assignments = problem_states.assignments;
      const auto &patterns = problem_states.patterns;
      graph_generator->set_grounded_problem_and_pattern(problem, patterns);
      for (const planning::Assignment &assign : assignments) {
        for (auto &graph : graph_generator->to_graphs(assign)) {
          graphs.push_back(*graph);
        }
      }
    }

    return graphs;
  }
}  // namespace data

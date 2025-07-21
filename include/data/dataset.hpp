#ifndef DATA_DATASET_HPP
#define DATA_DATASET_HPP

#include "../graph/graph.hpp"
#include "../graph/ilg_generator.hpp"
#include "../planning/problem.hpp"
#include "../planning/state.hpp"
#include "../planning/abstract.hpp"

#include <memory>
#include <string>
#include <vector>

namespace data {
  class ProblemStates {
   public:
    const planning::Problem problem;
    const std::vector<planning::State> states;

    ProblemStates(const planning::Problem &problem, const std::vector<planning::State> &states)
        : problem(problem), states(states){};
  };

  class ProblemPatternsAssignments {
   public:
    const planning::GroundedProblem problem;
    const planning::Patterns patterns;
    const std::vector<planning::Assignment> assignments;

    ProblemPatternsAssignments(const planning::GroundedProblem &problem, 
                          const planning::Patterns &patterns, 
                          const std::vector<planning::Assignment> &assignements)
        : problem(problem), patterns(patterns), assignments(assignements) {};
  };

  class Dataset {
   public:
    const planning::Domain &domain;

    Dataset(const planning::Domain &domain);
    virtual ~Dataset() = default;

    virtual size_t get_size() const = 0;
    virtual std::vector<graph::Graph> get_graphs(std::shared_ptr<graph::GraphGenerator> graph_generator) const = 0;

    planning::Domain get_domain() const { return domain; }
  };

  class LiftedDataset : public Dataset {    
   public:
    LiftedDataset(const planning::Domain &domain, const std::vector<ProblemStates> &data);

    size_t get_size() const override;
    std::vector<graph::Graph> get_graphs(std::shared_ptr<graph::GraphGenerator> graph_generator) const override;
   private:
    const std::vector<ProblemStates> data;
    std::unordered_map<std::string, int> predicate_to_arity;

    void check_good_atom(const planning::Atom &atom,
                         const std::unordered_set<planning::Object> &objects) const;
  };

  class GroundedDataset : public Dataset {
    const std::vector<ProblemPatternsAssignments> data;
   public:
    GroundedDataset(const planning::Domain &domain,
                    const std::vector<ProblemPatternsAssignments> &data);

    size_t get_size() const override;
    std::vector<graph::Graph> get_graphs(std::shared_ptr<graph::GraphGenerator> graph_generator) const override;

    std::vector<ProblemPatternsAssignments> get_data() const { return data; }
  };
}  // namespace data

#endif  // DATA_DATASET_HPP

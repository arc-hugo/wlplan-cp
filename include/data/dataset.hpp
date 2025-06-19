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

  class GroundedProblemStates {
   public:
    const planning::GroundedProblem problem;
    const planning::Patterns &patterns;
    const std::vector<planning::Assignment> assignements;

    GroundedProblemStates(const planning::GroundedProblem &problem, 
                          const planning::Patterns &patterns, 
                          const std::vector<planning::Assignment> &states)
        : problem(problem), patterns(patterns), assignements(assignements){};
  };

  template <typename T> class Dataset {
   public:
    const planning::Domain &domain;
    const std::vector<T> &data;

    Dataset(const planning::Domain &domain, const std::vector<T> &data);

    virtual size_t get_size() const = 0;
    virtual std::vector<graph::Graph> get_graphs(std::shared_ptr<graph::GraphGenerator> graph_generator) const = 0;

    virtual std::vector<T> get_data() const { return data; };
  };

  class LiftedDataset : public Dataset<ProblemStates> {
    LiftedDataset(const planning::Domain &domain, const std::vector<ProblemStates> &data);

    size_t get_size() const override;
    std::vector<graph::Graph> get_graphs(std::shared_ptr<graph::GraphGenerator> graph_generator) const override;
   private:
    std::unordered_map<std::string, int> predicate_to_arity;

    void check_good_atom(const planning::Atom &atom,
                         const std::unordered_set<planning::Object> &objects) const;
  };

  class GroundedDataset : public Dataset<GroundedProblemStates> {
    GroundedDataset(const planning::Domain &domain,
                    const std::vector<GroundedProblemStates> &data);

    size_t get_size() const override;
    std::vector<graph::Graph> get_graphs(std::shared_ptr<graph::GraphGenerator> graph_generator) const override;

  };
}  // namespace data

#endif  // DATA_DATASET_HPP

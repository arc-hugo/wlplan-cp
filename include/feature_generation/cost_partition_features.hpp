#ifndef FEATURE_GENERATION_COST_PARTITION_FEATURES_HPP
#define FEATURE_GENERATION_COST_PARTITION_FEATURES_HPP

#include "../graph/cplg_generator.hpp"
#include "features.hpp"
#include "generator.hpp"

#include <string>
#include <vector>
#include <coroutine>

namespace feature_generation {
  using CostPartition = std::vector<std::vector<double>>;

  class CostPartitionFeatures : public Features {
    std::vector<planning::Action> actions;
   private:
    std::unordered_map<std::string, std::vector<Embedding>> _embed_assign(const planning::Assignment &assignment);
    std::unordered_map<std::string, std::vector<Embedding>> _embed_graphs(const std::vector<std::shared_ptr<graph::Graph>> &graphs);
    std::unordered_map<std::string, Embedding> _embed(const std::shared_ptr<graph::Graph> &graph, const int graph_id);

   public:
    CostPartitionFeatures(const std::string feature_name,
                  const planning::Domain &domain,
                  std::string graph_representation,
                  int iterations,
                  std::string pruning,
                  bool multiset_hash,
                  PredictionTask task);

    CostPartitionFeatures(const std::string &filename);

    virtual std::unordered_map<std::string, Embedding> subgraph_embed_impl(
      const std::shared_ptr<graph::Graph> &graph,
      const int graph_id) = 0;
    
    // overloaded dataset embedding function
    Couroutine_Generator<std::unordered_map<std::string, std::vector<feature_generation::Embedding>>> 
    embed_dataset(const data::GroundedDataset &dataset);

    // flatten vector of all embeddings in the order of operators
    Embedding get_flattened_embeddings(const planning::Assignment &assignment,
                                       const std::vector<std::string> &operators);

    void set_grounded_problem_and_pattern(const planning::GroundedProblem &problem,
                                          const planning::Patterns &patterns);

  };
}  // namespace feature_generation

#endif  // FEATURE_GENERATION_COST_PARTITION_FEATURES_HPP

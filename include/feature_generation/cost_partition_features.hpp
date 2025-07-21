#ifndef FEATURE_GENERATION_COST_PARTITION_FEATURES_HPP
#define FEATURE_GENERATION_COST_PARTITION_FEATURES_HPP

#include "../graph/cplg_generator.hpp"
#include "features.hpp"

#include <string>
#include <vector>
#include <coroutine>
#include <generator>

namespace feature_generation {
  using CostPartition = std::vector<std::vector<double>>;

  class CostPartitionFeatures : public Features {
    std::vector<planning::Action> actions;
   public:
    CostPartitionFeatures(const std::string feature_name,
                  const planning::Domain &domain,
                  std::string graph_representation,
                  int iterations,
                  std::string pruning,
                  bool multiset_hash,
                  PredictionTask task);

    CostPartitionFeatures(const std::string &filename);

    virtual std::unordered_map<std::string, Embedding> actions_embed_impl(
      const std::shared_ptr<graph::Graph> &graph,
      const int graph_id) = 0;
    
      // overloaded embedding functions
    std::generator<std::unordered_map<std::string, std::vector<Embedding>>> actions_embed_dataset(const data::GroundedDataset &dataset);
    std::unordered_map<std::string, std::vector<Embedding>> actions_embed_assignment(const planning::Assignment &assignment);
    std::unordered_map<std::string, std::vector<Embedding>> actions_embed_graphs(const std::vector<std::shared_ptr<graph::Graph>> &graphs);
    std::unordered_map<std::string, Embedding> actions_embed_graph(const graph::Graph &graph, const int graph_id);
    std::unordered_map<std::string, Embedding> actions_embed(const std::shared_ptr<graph::Graph> &graph, const int graph_id);

    CostPartition predict_cost_partition(const std::vector<std::shared_ptr<graph::Graph>> &graphs);
    CostPartition predict_cost_partition(const planning::Assignment &assignment);
    
    void set_grounded_problem_and_pattern(const planning::GroundedProblem &problem,
                                          const planning::Patterns &patterns);

  };
}  // namespace feature_generation

#endif  // FEATURE_GENERATION_COST_PARTITION_FEATURES_HPP

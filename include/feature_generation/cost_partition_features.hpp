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

  enum EmbedType {
    GraphActions,
    Actions
  };

  class CostPartitionFeatures : public Features {
    std::vector<planning::Action> actions;
   private:
    std::generator<std::unordered_map<std::string, std::vector<Embedding>>> _embed_dataset(const data::GroundedDataset &dataset, const EmbedType type);
    std::unordered_map<std::string, std::vector<Embedding>> _embed_assignment(const planning::Assignment &assignment, const EmbedType type);
    std::unordered_map<std::string, std::vector<Embedding>> _embed_graphs(const std::vector<std::shared_ptr<graph::Graph>> &graphs, const EmbedType type);
    std::unordered_map<std::string, Embedding> _embed(const std::shared_ptr<graph::Graph> &graph, const int graph_id, const EmbedType type);

   public:
    CostPartitionFeatures(const std::string feature_name,
                  const planning::Domain &domain,
                  std::string graph_representation,
                  int iterations,
                  std::string pruning,
                  bool multiset_hash,
                  PredictionTask task);

    CostPartitionFeatures(const std::string &filename);

    virtual std::unordered_map<std::string, Embedding> graph_and_actions_embed_impl(
      const std::shared_ptr<graph::Graph> &graph,
      const int graph_id) = 0;
    
    virtual std::unordered_map<std::string, Embedding> actions_embed_impl(
      const std::shared_ptr<graph::Graph> &graph,
      const int graph_id) = 0;

    // overloaded graph embedding functions
    std::generator<std::unordered_map<std::string, std::vector<Embedding>>> actions_embed_dataset(const data::GroundedDataset &dataset);
    // overloaded action + graph embedding functions
    std::generator<std::unordered_map<std::string, std::vector<Embedding>>> graph_and_actions_embed_dataset(const data::GroundedDataset &dataset);

    CostPartition predict_cost_partition(const std::vector<std::shared_ptr<graph::Graph>> &graphs);
    CostPartition predict_cost_partition(const planning::Assignment &assignment);
    
    void set_grounded_problem_and_pattern(const planning::GroundedProblem &problem,
                                          const planning::Patterns &patterns);

  };
}  // namespace feature_generation

#endif  // FEATURE_GENERATION_COST_PARTITION_FEATURES_HPP

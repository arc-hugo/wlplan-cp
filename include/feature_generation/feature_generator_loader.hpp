#ifndef FEATURE_GENERATION_FEATURE_GENERATOR_LOADER_HPP
#define FEATURE_GENERATION_FEATURE_GENERATOR_LOADER_HPP

#include "features.hpp"
#include "cost_partition_features.hpp"

#include <memory>
#include <string>

std::shared_ptr<feature_generation::Features> load_feature_generator(const std::string save_file);
std::shared_ptr<feature_generation::CostPartitionFeatures> load_cost_partition_feature_generator(const std::string save_file);

#endif  // FEATURE_GENERATION_FEATURE_GENERATOR_LOADER_HPP

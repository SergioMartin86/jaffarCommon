#pragma once

/**
 * @file concurrent.hpp
 * @brief Containers designed for fast parallel, mutual exclusive access
 */

#include <mutex>
#include "../extern/phmap/parallel_hashmap/phmap.h"

namespace jaffarCommon
{

template <class V> using HashSet_t = phmap::parallel_flat_hash_set<V, phmap::priv::hash_default_hash<V>, phmap::priv::hash_default_eq<V>, std::allocator<V>, 4, std::mutex>;
template <class K, class V> using HashMap_t = phmap::parallel_flat_hash_map<K, V, phmap::priv::hash_default_hash<K>, phmap::priv::hash_default_eq<K>, std::allocator<std::pair<const K, V>>, 4, std::mutex>;

}
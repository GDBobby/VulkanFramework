#include "EightWinds/Backend/Descriptors/Binding.h"

#include <numeric>

#include <algorithm>

namespace EWE{
    namespace Descriptor{
        void Bindings::Sort() noexcept{
            std::size_t n = b.vkBindings.size();

            // 1. Build index permutation
            std::vector<std::size_t> idx(n);
            std::iota(idx.begin(), idx.end(), 0);

            std::sort(idx.begin(), idx.end(),
                [&](size_t a, size_t b2) {
                    return b.vkBindings[a].binding < b.vkBindings[b2].binding;
                }
            );

            // 2. Apply permutation in-place
            std::vector<bool> visited(n, false);

            for (size_t i = 0; i < n; ++i)
            {
                if (visited[i])
                    continue;

                size_t current = i;
                while (!visited[current])
                {
                    visited[current] = true;
                    size_t next = idx[current];

                    if (next == current)
                        break;

                    // Swap BOTH arrays
                    std::swap(b.vkBindings[current], b.vkBindings[next]);
                    std::swap(b.writes[current],     b.writes[next]);

                    current = next;
                }
            }
        }
    }
}
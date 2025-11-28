#include "EightWinds/Backend/Descriptor/Binding.h"

#include <numeric>

#include <algorithm>

namespace EWE{
    namespace Backend{
        namespace Descriptor{
            void Bindings::Sort() noexcept{
                std::size_t n = vkBindings.size();

                std::vector<std::size_t> idx(n);
                std::iota(idx.begin(), idx.end(), 0);

                std::sort(idx.begin(), idx.end(),
                    [&](std::size_t a, std::size_t b2) {
                        return vkBindings[a].binding < vkBindings[b2].binding;
                    }
                );

                std::vector<bool> visited(n, false);

                for (std::size_t i = 0; i < n; ++i) {
                    if (visited[i]) {
                        continue;
                    }

                    std::size_t current = i;
                    while (!visited[current]) {
                        visited[current] = true;
                        std::size_t next = idx[current];

                        if (next == current) {
                            break;
                        }

                        std::swap(vkBindings[current], vkBindings[next]);
                        bool temp = writes[current];
                        writes[current] = writes[next];
                        writes[next] = temp;

                        current = next;
                    }
                }
            }
        } //namespace Descriptor
    } //namespace Backend
} //namespace EWE
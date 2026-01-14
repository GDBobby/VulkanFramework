#pragma once

#include <cstdint>

namespace EWE{
	template<typename T>
	struct Color_RGB {
		T red;
		T green;
		T blue;
	};
	template<typename T>
	struct Color_RGBA {
		T red;
		T green;
		T blue;
		T alpha;
	};
}
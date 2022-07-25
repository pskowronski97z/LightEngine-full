#include <GI_Tools.h>

namespace GI {

	namespace RSM {

		std::vector<float> generate_sampling_pattern(const uint32_t samples_count) {

			const uint32_t size = 4 * sizeof(float) * samples_count;
			const float pi_x2 = 2.0 * 3.1415926;

			std::vector<float> result(size, 0.0);
			std::random_device rd;
			std::mt19937 rng(rd());
			std::uniform_real_distribution urd(0.0, 1.0);

			for (uint32_t i = 0; i < size; i += 4) {

				float e1 = urd(rng);
				float e2 = urd(rng);
			
				result[i] = e1 * sin(pi_x2 * e2);
				result[i + 1u] = e1 * cos(pi_x2 * e2);
				result[i + 2u] = e1 * e1;
			}

			return result;
		}
	}
}


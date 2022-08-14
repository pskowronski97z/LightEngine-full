#include <GI_Tools.h>

namespace GI {

	namespace RSM {
		// 1. R, dAngle - uniform
		// 2. R - normalized, dAngle - uniform
		// 3. R - normalized, dAngle - constant
		// 4. R, dAngle - Fibonacci Sunflower Pattern
		std::vector<float> generate_sampling_pattern(const uint32_t samples_count, const uint8_t option) {

			const uint32_t size = 4u * samples_count;
			const float pi = atan(1) * 4.0f, 
						dAngle = 1.0f / samples_count,
						goldenAngle = 137.5f * pi / 180.0f,
						fib_norm = sqrt(samples_count),
						double_pi = pi * 2.0f;
			float e1, 
				  e2;
			std::vector<float> result(size, 0.0f);
			std::random_device rd;
			std::mt19937 rng(rd());
			std::uniform_real_distribution<float> urd(0.0f, 1.0f);
			std::normal_distribution<> nd(0.7f, 0.30f);
					
			for (uint32_t i=0u, n = 1u; i < size; i += 4u, n++) {
				
				switch (option) {
				case 1u:
					e1 = urd(rng);
					e2 = urd(rng);

					result[i] = e1 * cos(double_pi * e2);
					result[i + 1u] = e1 * sin(double_pi * e2);
					result[i + 2u] = e1 * e1;

					break;
				
				case 2u:
					e1 = std::min(nd(rng), 1.0);
					e2 = urd(rng);

					result[i] = e1 * cos(double_pi * e2);
					result[i + 1u] = e1 * sin(double_pi * e2);
					result[i + 2u] = 1.0f;

					break;

				case 3u:

					e1 = std::min(nd(rng), 1.0);
					e2 = dAngle * n;

					result[i] = e1 * cos(double_pi * e2);
					result[i + 1u] = e1 * sin(double_pi * e2);
					result[i + 2u] = 1.0f;

					break;

				case 4u:

					e1 = sqrt(n)/fib_norm;
					e2 = goldenAngle * n;

					result[i] = e1 * cos(e2);
					result[i + 1u] = e1 * sin(e2);
					result[i + 2u] = 1.0f;

					break;	

				default:
					break;
				}			
			}

			return result;
		}
	
		std::vector<float> generate_random_floats(const uint32_t size) {

			std::vector<float> result(size, 0.0f);
			std::random_device rd;
			std::mt19937 rng(rd());
			std::uniform_real_distribution<float> urd(0.0f, 1.0f);

			for (auto& val : result)
				val = urd(rng);

			return result;
		}
	}
}


#include <stdio.h>
#include <gmp.h> // Ramanujan's Pi


// Also can be called Babylonian Method
double newton_sqrt_manual(const double x, int iterations) {
	if (x < 0) {
		fprintf(stderr, "Error: Keep it real! newton_sqrt() got: %.2f as x.\n", x);
		return -1.0;
	}

	double guess = x / 2.0;
	int i;
	for (i = 0; i < iterations; i++) {
		guess = 0.5 * (guess + (x / guess));
	}
	return guess;
}


// === Trigonometry functions ===

void ramanujans_pi(const unsigned int k, const unsigned int precision, mpf_t result) {
	// NOTE: RETURNS 1 / pi, NOT pi itself!
	// Magic values in the code come from 
	// Srinivasa Ramanujan formula for pi

	// Init Integers
	mpz_t numerator, denominator, temp, temp2;
	mpz_inits(numerator, denominator, temp, temp2, NULL);

	// Init Floats
	mpf_t constant, term, num_f, den_f;
	mpf_init2(constant, precision);
	mpf_init2(term, precision);
	mpf_init2(num_f, precision);
	mpf_init2(den_f, precision);

	// Set result to zero
	mpf_set_ui(result, 0);

	// constant = (sqrt(2) * 2) / 9801
	mpf_t two, ninety_hundred;
	mpf_init2(two, precision);
	mpf_init2(ninety_hundred, precision);
	mpf_set_ui(two, 2);
	mpf_set_ui(ninety_hundred, 9801);

	mpf_sqrt(constant, two); // const = sqrt(2)
	mpf_mul_ui(constant, constant, 2); // const = const * 2
	mpf_div(constant, constant, ninety_hundred); // const = const / 9801

	// Free variables
	mpf_clears(two, ninety_hundred,  NULL);

	// Main sum loop
	for (unsigned int i = 0; i < k; i++) {
		mpz_set_ui(numerator, 0);
		mpz_set_ui(denominator, 0);
		mpf_set_ui(term, 0);

		// numerator = (fact(4 * i)) * (1103 + (26390 * i))
		mpz_fac_ui(numerator, (4 * i));
		mpz_set_ui(temp, 26390);
		mpz_mul_ui(temp, temp, i); // temp = temp * i
		mpz_add_ui(temp, temp, 1103); // temp = temp + 1103
		mpz_mul(numerator, numerator, temp); // left part * right part

		// denominator = pow(fact(i), 4)) * pow(396, (4 * i))
		mpz_fac_ui(denominator, i); // i!
		mpz_pow_ui(denominator, denominator, 4); // (i!)^4
		mpz_set_ui(temp2, 396);
		mpz_pow_ui(temp2, temp2, (4 * i)); // pow(396, 4 * i)
		mpz_mul(denominator, denominator, temp2); // left part * right part

		// term = numerator / denominator
		mpf_set_z(num_f, numerator);
		mpf_set_z(den_f, denominator);
		mpf_div(term, num_f, den_f);

		// result = result + term
		mpf_add(result, result, term);

	}

	// Final Result = constant / final result
	mpf_mul(result, constant, result);

	// if you want pi instead of inverse pi
	// Just divide one by Final Result
	// pi = 1 / result

	// Clear variables
	mpz_clears(numerator, denominator, temp, temp2, NULL);
	mpf_clears(constant, term, num_f, den_f, NULL);

}

// Funcs with hard-coded pi values
// Ugly but work, and is enough for most practical use cases
static inline float return_float_pi() {
	return 3.14159265f;
}
static inline double return_double_pi() {
	return 3.1415926535897932;
}

static inline float deg_to_rad(const float degrees) {
	float radians = degrees * (3.14159265f / 180);
	return radians;
}

static inline float rad_to_deg(const float radians) {
	float degrees = radians * (180 / 3.14159265f);
	return degrees;
}


// == Mathematical Utilities ==

static inline float clamp_f(const float value, const float min, const float max) {
	if (value < min) return min;
	if (value > max) return max;
	return value;
}

static inline int clamp_i(const int value, const int min, const int max) {
	if (value < min) return min;
	if (value > max) return max;
	return value;
}

// Linear Interpolation
// lerp(a, b, t) = a + (b - a) * t
// For example: If t = 0 => return a; If t = 1 => return b
static inline float lerp(const float a, const float b, const float t) {
	return a + ((b - a) * t);
}

static inline int sign_i(const int x) {
	if (x > 0) return 1; // +
	if (x < 0) return -1; // -
	return 0; // 0 == 0
}

static inline float sign_f(const float x) {
	if (x > 0) return 1; // +
	if (x < 0) return -1; // -
	return 0; // 0 == 0
}

static inline float abs_f(const float value) {
	union {
		float value;
		unsigned int i;
	} u; // [sign 1b][exp 8b][mantissa 23b]
	u.value = value;
	u.i &= 0x7FFFFFFF; // mask sign
	return u.value;
}

static inline int abs_i(const int value) {
	int mask = value >> 31; // funny
	return (value ^ mask) - mask;
}

float newton_sqrt_f(const float x) {
	if (x < 0) {
		fprintf(stderr, "Error: Keep it real! newton_sqrt() got: %.2f as x.\n", x);
		return -1.0f;
	}
	if (x == 0) return 0.0f;

	float guess = x / 2.0f;
	float epsilon = 1e-6f; // acceptable precision deviation
	float prev;

	do {
		prev = guess;
		guess = 0.5f * (prev + (x / prev));
	} while (abs_f(guess - prev) > epsilon);

	return guess;
}

static inline int math_floor(const float x) {
	int i = (int)x;
	return (x < 0 && x != (float)i) ? (i - 1) : i;

}

static inline int math_ceil(const float x) {
	int i = (int)x;
	return (x > 0 && x != (float)i) ? (i + 1) : i;
}

static inline int math_mod(const int x, const int y) {
	int r = x % y;
	return (r < 0) ? (r + (y > 0 ? y : -y)) : r;
}
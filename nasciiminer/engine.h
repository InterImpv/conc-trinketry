#include "perlin.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

class point32_t {
public:
	int32_t x, y;

	point32_t(void);
	point32_t(const uint32_t x, const uint32_t y);
	point32_t(const point32_t &p);
	~point32_t(void) = default;
};

class engine_t {
private:
	int32_t key;
	point32_t scrsize;
	
	uint64_t map_w;
	uint64_t map_h;
	uint64_t map_z;
	uint64_t currlayer;
	uint64_t layers;
	std::vector<std::vector<std::string>> map;
	PerlinNoise perlin;

	/* private functions */
	void map_generate_layer(void);
	void map_generate_fill(const char ch);
	void render_map(void);

	void inc_layer(void);
	void dec_layer(void);

public:
	engine_t(void);
	~engine_t(void) = default;

	void map_generate(const uint64_t w, const uint64_t h, const uint64_t z);

	void render_frame(void);
	void simulate_frame(void);
};
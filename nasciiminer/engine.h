
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

	std::vector<std::string> map;

	/* private functions */
	void render_map(void);

public:
	engine_t(void);
	~engine_t(void) = default;

	void map_generate(uint64_t size);

	void render_frame(void);
	void simulate_frame(void);
};
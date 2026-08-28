#include <atomic>
#include <cstdint>
#include <new>
#include <optional>
#include <bit>
#include <memory>

const uint64_t CAPACITY = 4096;

struct RawMessage {
	char data[131072];
	uint64_t length;
};

// custom torus queue from https://github.com/kobebayliss/torus
template<typename T, uint64_t capacity>
class SPSCQueue {
	static_assert(std::is_trivially_copyable_v<T>, "SPSCQueue requires trivially copyable object");
	T* buffer;  // align to 64 byte segments for cache friendliness
	static constexpr uint64_t rounded_capacity = std::bit_ceil(capacity);  // round to power of two for one cycle position calculation

	alignas(std::hardware_destructive_interference_size) std::atomic<uint64_t> head = 0;
	alignas(std::hardware_destructive_interference_size) std::atomic<uint64_t> tail = 0;

public:
	uint64_t tail_cached = 0;
	uint64_t head_cached = 0;
	SPSCQueue() {
		buffer = static_cast<T*>(std::aligned_alloc(64, rounded_capacity * sizeof(T)));
		if (!buffer) throw std::bad_alloc();
	}
	~SPSCQueue () { std::free(buffer); }
	SPSCQueue (const SPSCQueue& other) = delete;
	SPSCQueue& operator=(const SPSCQueue& other) = delete;
	SPSCQueue (SPSCQueue&& other) = delete;
	SPSCQueue& operator=(SPSCQueue&& other) = delete;

	[[nodiscard]] bool try_push(const T& item) noexcept {
		uint64_t h = head.load(std::memory_order_relaxed);
		if (h - tail_cached == rounded_capacity) {
			tail_cached = tail.load(std::memory_order_acquire);
			if (h - tail_cached == rounded_capacity) return false;
		}

		buffer[h & (rounded_capacity - 1)] = item;
		head.store(h + 1, std::memory_order_release);
		return true;
	}
	[[nodiscard]] std::optional<T> try_pop() noexcept {
		uint64_t t = tail.load(std::memory_order_relaxed);
		if (t == head_cached) {
			head_cached = head.load(std::memory_order_acquire);
			if (t == head_cached) {
				return std::nullopt;
			}
		}
		T item = buffer[t & (rounded_capacity - 1)];
		tail.store(t + 1, std::memory_order_release);
		return item;
	}
};

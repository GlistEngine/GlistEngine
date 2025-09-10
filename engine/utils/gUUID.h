//
// Created by Metehan Gezer on 14/09/2025.
//

#ifndef UTILS_GUUID_H
#define UTILS_GUUID_H

#include <array>
#include <cstdint>
#include <string_view>

class gUUID {
public:
	using bytes = std::array<std::uint8_t, 16>;

	gUUID() noexcept;
	explicit gUUID(const bytes& b) noexcept;

	static gUUID generateV4();

	static gUUID fromString(std::string_view s, bool* ok = nullptr) noexcept;

	std::string toString() const;

	bytes toBytes() const noexcept;

	bool isNil() const noexcept { return hi == 0 && lo == 0; }

	friend bool operator==(const gUUID& a, const gUUID& b) noexcept {
		return a.hi == b.hi && a.lo == b.lo;
	}

	friend bool operator!=(const gUUID& a, const gUUID& b) noexcept {
		return !(a == b);
	}

	friend bool operator<(const gUUID& a, const gUUID& b) noexcept {
		return a.hi < b.hi || (a.hi == b.hi && a.lo < b.lo);
	}


	std::uint64_t getHigh() const noexcept { return hi; }

	std::uint64_t getLow() const noexcept { return lo; }

private:
	std::uint64_t hi;
	std::uint64_t lo;

	void fromBytes(const bytes& b) noexcept;

};


// hashing support
namespace std {
template<> struct hash<gUUID> {
	size_t operator()(const gUUID& u) const noexcept {
		// 64-bit mix (xorshift64*)
		std::uint64_t x = u.getHigh() ^ (u.getLow() * 0x9E3779B97F4A7C15ull);
		x ^= x >> 30; x *= 0xBF58476D1CE4E5B9ull;
		x ^= x >> 27; x *= 0x94D049BB133111EBull;
		x ^= x >> 31;
		return x;
	}
};
}

#endif//UTILS_GUUID_H

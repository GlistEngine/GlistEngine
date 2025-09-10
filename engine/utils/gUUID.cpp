//
// Created by Metehan Gezer on 14/09/2025.
//

#include "gUUID.h"
#include <random>

static std::mt19937_64::result_type seedRNG() {
  // gather multiple 32-bit entropy chunks to seed a 64-bit engine well
  std::random_device rd;
  std::seed_seq seq{rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd()};
  return std::mt19937_64(seq)();  // extract a seeded state
}

static void writeU64BE(std::uint8_t* out, std::uint64_t v) noexcept {
  for (int i = 7; i >= 0; --i) {
    out[7 - i] = static_cast<std::uint8_t>((v >> (i * 8)) & 0xFF);
  }
}

static std::uint64_t readU64BE(const std::uint8_t* in) noexcept {
  std::uint64_t v = 0;
  for (int i = 0; i < 8; ++i) {
    v = v << 8 | in[i];
  }
  return v;
}

gUUID::gUUID() noexcept : hi(0), lo(0) {}

gUUID::gUUID(const bytes& b) noexcept {
  fromBytes(b);
}

gUUID gUUID::generateV4() {
  // use std::random_device to seed, then 64-bit engine
  static thread_local std::mt19937_64 rng{seedRNG()};
  std::uniform_int_distribution<std::uint64_t> dist;

  gUUID u;
  std::uint64_t hi = dist(rng);
  std::uint64_t lo = dist(rng);

  // set version (4) -> high 4 bits of byte 6 (zero-based)
  // set variant (10xx) -> high 2 bits of byte 8
  bytes b{};
  writeU64BE(b.data(), hi);
  writeU64BE(b.data() + 8, lo);

  b[6] = static_cast<std::uint8_t>((b[6] & 0x0F) | 0x40);  // version 4
  b[8] = static_cast<std::uint8_t>((b[8] & 0x3F) | 0x80);  // variant RFC 4122

  u.fromBytes(b);
  return u;
}

gUUID gUUID::fromString(std::string_view s, bool* ok) noexcept {
  // accepted forms: 8-4-4-4-12 with lowercase/uppercase hex
  gUUID out;
  if (s.size() != 36 || s[8] != '-' || s[13] != '-' || s[18] != '-' ||
      s[23] != '-') {
    if (ok)
      *ok = false;
    return out;
  }
  bytes b{};
  auto hex = [&](char c) -> int {
    if (c >= '0' && c <= '9')
      return c - '0';
    if (c >= 'a' && c <= 'f')
      return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F')
      return 10 + (c - 'A');
    return -1;
  };
  int bi = 0;
  for (size_t i = 0; i < s.size();) {
    if (s[i] == '-') {
      ++i;
      continue;
    }
    if (i + 1 >= s.size()) {
      if (ok)
        *ok = false;
      return out;
    }
    int h = hex(s[i]), l = hex(s[i + 1]);
    if (h < 0 || l < 0) {
      if (ok)
        *ok = false;
      return out;
    }
    b[bi++] = static_cast<std::uint8_t>((h << 4) | l);
    i += 2;
  }
  if (bi != 16) {
    if (ok)
      *ok = false;
    return out;
  }
  out.fromBytes(b);
  if (ok)
    *ok = true;
  return out;
}

std::string gUUID::toString() const {
  static constexpr char kHex[] = "0123456789abcdef";
  bytes b = toBytes();
  std::string s;
  s.resize(36);
  int j = 0;
  for (int i = 0; i < 16; ++i) {
    if (i == 4 || i == 6 || i == 8 || i == 10)
      s[j++] = '-';
    s[j++] = kHex[b[i] >> 4];
    s[j++] = kHex[b[i] & 0x0F];
  }
  return s;
}

gUUID::bytes gUUID::toBytes() const noexcept {
  bytes b{};
  writeU64BE(b.data(), hi);
  writeU64BE(b.data() + 8, lo);
  return b;
}

void gUUID::fromBytes(const bytes& b) noexcept {
  hi = readU64BE(b.data());
  lo = readU64BE(b.data() + 8);
}
/*
 * This file is part of MinimalBoilerplateSerializationLibrary.
 * Copyright (C) 2026 Alexander Shaffer <alexander.shaffer.623@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

import std;
import mbsl;

/*
 * Custom Serialization/Deserialization Functions Example:
 *
 * All custom functions must be defined within the definition of mbsl::explicit_serializer.
 */
template<mbsl::instance_of_dynamic_serializer DynamicSerializer>
struct mbsl::explicit_serializer<DynamicSerializer> {
  // This function serializes std::vector. The serialized data is inserted into dynamic_serializer.
  // TODO: Implement this function.
  template<typename T>
  static void serialize(DynamicSerializer& dynamic_serializer, const std::vector<T>& vector) {}

  // This function deserializes any type. The deserialized data is returned as a newly allocated object.
  // TODO: Implement this function and constrain T to only allow instances of std::vector.
  template<typename T>
  static T deserialize(const std::span<std::byte> span) {
    return {};
  }
};

void print_span(const std::string_view desc, const std::span<const std::uint8_t> data, const std::size_t size) {
  std::println("{} = {} | {} size = {}", desc, data, desc, size);
}

void print_package(const auto& package) {
  print_span("package", std::span{reinterpret_cast<const std::uint8_t*>(&package), sizeof(package)}, sizeof(package));
}

void print_depot(const auto& depot) { print_span("depot", std::span{depot.template data<std::uint8_t>(), depot.size()}, sizeof(depot)); }

/*
 * This program will output the following on little-endian CPUs:
 *
 * Before serialize_mutably():
 * package = [8, 7, 6, 5, 4, 3, 2, 1, 4, 3, 2, 1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0] | package size = 32
 * After serialize_mutably():
 * package = [1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0] | package size = 32
 * depot = [1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0] | depot size = 8
 * Before deserialize_immutably():
 * depot = [1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0] | depot size = 8
 * After deserialize_immutably():
 * depot = [1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0] | depot size = 8
 * package = [8, 7, 6, 5, 4, 3, 2, 1, 4, 3, 2, 1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0] | package size = 32
 */
int main() {
  /*
   * A package is a struct defined in a .mbsl file.
   * Because C++23 lacks sufficient compile-time reflection, this library is only capable of serializing and deserializing packages.
   * The serialization/deserialization method is chosen at compile time depending on the package member types, CPU endianness, and whether or not
   * modifying the original data is acceptable. If modifying the original data is acceptable, then in-place serialization might be chosen, which
   * minimizes memory copying and allocation. This library offers three serialization/deserialization function types: mutably, immutably, and new.
   * These functions may be called in any combination. For example, serialize_mutably() can serialize a package, then deserialize_new() can
   * deserialize it back.
   */
  mbsl::example_group::simple_package simple_package{
    .x_ = std::bit_cast<double>(0x0102030405060708), .y_ = 0x01020304, .z_ = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}};

  /*
   * Serialization Example:
   *
   * All serialization functions return a depot object, which provide a common interface to access the serialized data.
   * In this example, serialize_mutably() is used, and simple_package is mutable and resides in contiguous memory.
   * Therefore, simple_package is serialized in-place, and the compiler simplifies the depot object into a pointer that points to the modified
   * package. serialize_mutably() can be replaced with serialized_immutably() or serialize_new(), which will change the outcome slightly:
   *
   * serialize_immutably(): This function never modifies the original package. Therefore, the serialized data is written to the depot object,
   *   and the compiler simplifies the depot object into an array of bytes. If no serialization is necessary, then the compiler would simplify
   *   the depot object into a pointer.
   *
   * serialize_new(): Similar to serialize_immutably() except memory is always allocated. This function is useful if the package can be deallocated
   *   before the serialized data is used.
   */

  std::println("Before serialize_mutably():");
  print_package(simple_package);

  const mbsl::depot serialized{mbsl::serialize_mutably(simple_package)};

  std::println("After serialize_mutably():");
  print_package(simple_package);
  print_depot(serialized);

  /*
   * Deserialization Example:
   *
   * In this example, deserialize_immutably() is used, which is similar to serialize_immutably() except data is deserialized and a new simple_package
   * object is created. If no deserialization is necessary, then the data is interpreted as a simple_package reference.
   *
   * Likewise, deserialize_mutably() and deserialize_new() behave similarly to their serialization
   * counterparts except they deserialize data and do not return a depot object.
   */
  std::println("Before deserialize_immutably():");
  print_depot(serialized);

  // TODO: Allow depots and arrays to be passed directly into deserialization functions.
  std::span span{serialized.data<std::byte>(), serialized.size()};

  // Use decltype(auto) here because deserialization functions may return a package reference or a newly constructed package.
  decltype(auto) deserialized{mbsl::deserialize_immutably<mbsl::example_group::simple_package>(span)};

  std::println("After deserialize_immutably():");
  print_depot(serialized);
  print_package(deserialized);

  return 0;
}

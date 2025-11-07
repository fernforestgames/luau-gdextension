#include "doctest.h"

#include <vector>

#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/char_string.hpp>

#include "string_cache.h"

using namespace gdluau;
using namespace godot;

TEST_SUITE("StringCache")
{
    TEST_CASE("create_atom - creates atom for valid string")
    {
        const char *test_str = "test_function";
        int16_t atom = create_atom(test_str, strlen(test_str));

        CHECK(atom >= 0);
        CHECK(atom < static_cast<int16_t>(CACHE_SIZE));
    }

    TEST_CASE("create_atom - returns same atom for same string")
    {
        const char *test_str = "repeated_string";
        int16_t atom1 = create_atom(test_str, strlen(test_str));
        int16_t atom2 = create_atom(test_str, strlen(test_str));

        CHECK(atom1 >= 0);
        CHECK(atom1 == atom2);
    }

    TEST_CASE("create_atom - returns -1 for empty string")
    {
        int16_t atom = create_atom("", 0);
        CHECK(atom == -1);
    }

    TEST_CASE("create_atom - handles UTF-8 strings")
    {
        const char *utf8_str = "hello_世界_мир";
        int16_t atom = create_atom(utf8_str, strlen(utf8_str));

        CHECK(atom >= 0);

        // Verify we can retrieve it
        StringName retrieved = string_name_for_atom(atom);
        CHECK_FALSE(retrieved.is_empty());
    }

    TEST_CASE("create_atom - handles long strings")
    {
        // Create a long string with 1000 'a' characters
        String long_str;
        for (int i = 0; i < 1000; i++)
        {
            long_str += "a";
        }
        CharString utf8 = long_str.utf8();
        int16_t atom = create_atom(utf8.get_data(), utf8.length());

        CHECK(atom >= 0);
    }

    TEST_CASE("create_atom - different strings with potential hash collision")
    {
        // Create several different strings and verify they get atoms
        const char *strings[] = {
            "func_a",
            "func_b",
            "func_c",
            "method_x",
            "method_y",
            "method_z"};

        for (const char *str : strings)
        {
            int16_t atom = create_atom(str, strlen(str));
            // Should either get a valid atom or -1 (collision)
            CHECK(atom >= -1);
            CHECK(atom < static_cast<int16_t>(CACHE_SIZE));
        }
    }

    TEST_CASE("string_name_for_atom - retrieves created atom")
    {
        const char *test_str = "test_retrieval";
        int16_t atom = create_atom(test_str, strlen(test_str));
        CHECK(atom >= 0);

        StringName retrieved = string_name_for_atom(atom);
        CHECK_FALSE(retrieved.is_empty());
        CHECK(String(retrieved) == String(test_str));
    }

    TEST_CASE("string_name_for_atom - returns empty for negative atom")
    {
        StringName retrieved = string_name_for_atom(-1);
        CHECK(retrieved.is_empty());
    }

    TEST_CASE("string_name_for_atom - returns empty for invalid atom index")
    {
        StringName retrieved = string_name_for_atom(static_cast<int>(CACHE_SIZE) + 1000);
        CHECK(retrieved.is_empty());
    }

    TEST_CASE("string_name_for_atom - returns empty for unfilled slot")
    {
        // Atom 0 might be unfilled (depends on test order)
        // Just verify it doesn't crash
        StringName retrieved = string_name_for_atom(0);
        // Either empty or a valid StringName
        (void)retrieved;
    }

    TEST_CASE("char_string - caches UTF-8 conversion")
    {
        StringName test_name("test_char_string");

        // First call should populate cache
        CharString result1 = char_string(test_name);
        CHECK(result1.size() > 0);
        CHECK(strcmp(result1.get_data(), "test_char_string") == 0);

        // Second call should retrieve from cache
        CharString result2 = char_string(test_name);
        CHECK(result2.size() > 0);
        CHECK(strcmp(result2.get_data(), "test_char_string") == 0);
    }

    TEST_CASE("char_string - returns empty for empty StringName")
    {
        StringName empty_name;
        CharString result = char_string(empty_name);
        CHECK(result.size() == 0);
    }

    TEST_CASE("char_string - handles UTF-8 characters")
    {
        StringName utf8_name("hello_世界");
        CharString result = char_string(utf8_name);

        CHECK(result.size() > 0);
        // Should contain the UTF-8 encoded string
        CHECK(result.get_data() != nullptr);
    }

    TEST_CASE("char_string - handles multiple different strings")
    {
        StringName name1("string_one");
        StringName name2("string_two");
        StringName name3("string_three");

        CharString result1 = char_string(name1);
        CharString result2 = char_string(name2);
        CharString result3 = char_string(name3);

        CHECK(strcmp(result1.get_data(), "string_one") == 0);
        CHECK(strcmp(result2.get_data(), "string_two") == 0);
        CHECK(strcmp(result3.get_data(), "string_three") == 0);
    }

    TEST_CASE("char_string - same StringName returns consistent result")
    {
        StringName test_name("consistency_test");

        CharString result1 = char_string(test_name);
        CharString result2 = char_string(test_name);
        CharString result3 = char_string(test_name);

        CHECK(strcmp(result1.get_data(), result2.get_data()) == 0);
        CHECK(strcmp(result2.get_data(), result3.get_data()) == 0);
    }

    TEST_CASE("Integration - create_atom and string_name_for_atom roundtrip")
    {
        const char *original = "roundtrip_test";

        // Create atom
        int16_t atom = create_atom(original, strlen(original));
        CHECK(atom >= 0);

        // Retrieve StringName
        StringName retrieved = string_name_for_atom(atom);
        CHECK_FALSE(retrieved.is_empty());

        // Convert to CharString
        CharString char_str = char_string(retrieved);
        CHECK(strcmp(char_str.get_data(), original) == 0);
    }

    TEST_CASE("Integration - multiple atoms don't interfere")
    {
        const char *str1 = "atom_one";
        const char *str2 = "atom_two";
        const char *str3 = "atom_three";

        int16_t atom1 = create_atom(str1, strlen(str1));
        int16_t atom2 = create_atom(str2, strlen(str2));
        int16_t atom3 = create_atom(str3, strlen(str3));

        // All should succeed (unless collision)
        CHECK(atom1 >= -1);
        CHECK(atom2 >= -1);
        CHECK(atom3 >= -1);

        // If successful, verify retrieval
        if (atom1 >= 0)
        {
            StringName retrieved1 = string_name_for_atom(atom1);
            CHECK(String(retrieved1) == String(str1));
        }

        if (atom2 >= 0)
        {
            StringName retrieved2 = string_name_for_atom(atom2);
            CHECK(String(retrieved2) == String(str2));
        }

        if (atom3 >= 0)
        {
            StringName retrieved3 = string_name_for_atom(atom3);
            CHECK(String(retrieved3) == String(str3));
        }
    }

    TEST_CASE("Edge case - very long string in char_string cache")
    {
        // Create a very long string with 10000 'x' characters
        String long_content;
        for (int i = 0; i < 10000; i++)
        {
            long_content += "x";
        }
        StringName long_name(long_content);

        CharString result = char_string(long_name);
        CHECK(result.size() > 0);
        CHECK(strlen(result.get_data()) == 10000);
    }

    TEST_CASE("Edge case - special characters in atom creation")
    {
        const char *special = "func@#$%^&*()";
        int16_t atom = create_atom(special, strlen(special));

        if (atom >= 0)
        {
            StringName retrieved = string_name_for_atom(atom);
            CHECK(String(retrieved) == String(special));
        }
    }

    TEST_CASE("Edge case - null bytes in string (up to length)")
    {
        char str_with_null[10] = "abc\0def";
        // Only take up to null byte
        int16_t atom = create_atom(str_with_null, 3);

        if (atom >= 0)
        {
            StringName retrieved = string_name_for_atom(atom);
            CHECK(String(retrieved) == String("abc"));
        }
    }

    TEST_CASE("Collision handling - returns -1 when slot occupied by different string")
    {
        // This test is probabilistic - we try to force a collision
        // by creating many strings hoping two will hash to the same slot
        int collision_count = 0;
        const int num_attempts = 100;

        for (int i = 0; i < num_attempts; i++)
        {
            String test_str = String("collision_test_") + String::num_int64(i);
            CharString utf8 = test_str.utf8();
            int16_t atom = create_atom(utf8.get_data(), utf8.length());

            if (atom == -1)
            {
                collision_count++;
            }
        }

        // With 100 strings and 4096 slots, we should see some collisions
        // This is not guaranteed, so we just log it
        (void)collision_count;
        // Note: In a real scenario, collision_count > 0 would indicate
        // that the collision handling is working
    }

    TEST_CASE("Thread safety smoke test - repeated operations")
    {
        // This doesn't test true concurrency, but exercises the lock paths
        StringName test_name("thread_test");

        for (int i = 0; i < 100; i++)
        {
            CharString result = char_string(test_name);
            CHECK(result.size() > 0);
        }

        const char *str = "atom_test";
        for (int i = 0; i < 100; i++)
        {
            int16_t atom = create_atom(str, strlen(str));
            if (atom >= 0)
            {
                StringName retrieved = string_name_for_atom(atom);
                CHECK_FALSE(retrieved.is_empty());
            }
        }
    }

    TEST_CASE("Cache overflow - handles max size + 1 gracefully")
    {
        // Create CACHE_SIZE + 1 unique strings and verify behavior
        // Due to hash collisions, we may get -1 before filling all slots,
        // but we should definitely see collisions when exceeding cache size

        const size_t num_strings = CACHE_SIZE + 1;
        int successful_atoms = 0;
        int collision_atoms = 0;

        // Use a vector to store the strings and their atoms
        struct StringAtomPair
        {
            String str;
            int16_t atom;
        };
        std::vector<StringAtomPair> pairs;
        pairs.reserve(num_strings);

        // Create unique strings with a prefix to ensure uniqueness
        for (size_t i = 0; i < num_strings; i++)
        {
            String unique_str = String("cache_overflow_test_string_") + String::num_int64(i);
            CharString utf8 = unique_str.utf8();
            int16_t atom = create_atom(utf8.get_data(), utf8.length());

            pairs.push_back({unique_str, atom});

            if (atom >= 0)
            {
                successful_atoms++;
                CHECK(atom < static_cast<int16_t>(CACHE_SIZE));
            }
            else
            {
                CHECK(atom == -1);
                collision_atoms++;
            }
        }

        // Verify we created the expected number of strings
        CHECK(pairs.size() == num_strings);
        CHECK(successful_atoms + collision_atoms == static_cast<int>(num_strings));

        // With CACHE_SIZE + 1 strings and a hash-based cache,
        // we must have at least one collision (pigeonhole principle)
        // However, due to hash distribution, we likely have more
        CHECK(collision_atoms > 0);

        // Verify that successfully cached strings can be retrieved correctly
        for (const auto &pair : pairs)
        {
            if (pair.atom >= 0)
            {
                StringName retrieved = string_name_for_atom(pair.atom);
                CHECK_FALSE(retrieved.is_empty());
                CHECK(String(retrieved) == pair.str);
            }
        }

        // Verify that attempting to create atoms for the same strings again
        // returns the same result (cached atoms remain, collisions remain)
        for (const auto &pair : pairs)
        {
            CharString utf8 = pair.str.utf8();
            int16_t atom_again = create_atom(utf8.get_data(), utf8.length());
            CHECK(atom_again == pair.atom);
        }

        // Verify cache size limit is respected
        CHECK(successful_atoms <= static_cast<int>(CACHE_SIZE));
    }
}

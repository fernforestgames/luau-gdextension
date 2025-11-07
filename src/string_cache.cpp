#include "string_cache.h"

#include <godot_cpp/templates/spin_lock.hpp>

using namespace gdluau;
using namespace godot;

struct CacheSlot
{
    StringName str_name;
    CharString char_str;
};

constexpr size_t CACHE_SIZE = 1 << 12; // 4096 entries
constexpr size_t CACHE_MASK = CACHE_SIZE - 1;
static_assert(CACHE_MASK <= INT16_MAX);

// 64 KB cache (excluding the string contents), assuming this size.
// For comparison, Godot's own StringName interning is 512 KB.
//
// Assert because we don't want the cache to accidentally balloon if the engine types change.
static_assert(sizeof(CacheSlot) <= 16);

static CacheSlot *cache = nullptr;
static SpinLock *cache_lock = nullptr;

void gdluau::initialize_string_cache()
{
    cache = memnew_arr(CacheSlot, CACHE_SIZE);
    cache_lock = memnew(SpinLock);
}

void gdluau::uninitialize_string_cache()
{
    if (cache_lock != nullptr)
    {
        cache_lock->lock();
    }

    if (cache != nullptr)
    {
        memdelete_arr(cache);
        cache = nullptr;
    }

    if (cache_lock != nullptr)
    {
        cache_lock->unlock();

        memdelete(cache_lock);
        cache_lock = nullptr;
    }
}

static unsigned slot_for_string_name(const StringName &p_str_name)
{
    return static_cast<unsigned>(p_str_name.hash()) & CACHE_MASK;
}

int16_t gdluau::create_atom(const char *p_str, size_t p_len)
{
    ERR_FAIL_COND_V_MSG(p_len == 0, -1, "Cannot create atom for empty string.");

    StringName str_name(String::utf8(p_str, p_len));

    unsigned atom = slot_for_string_name(str_name);
    bool slot_filled;

    cache_lock->lock();
    {
        StringName &slot = cache[atom].str_name;
        if (slot.is_empty())
        {
            slot = std::move(str_name);
            slot_filled = true;
        }
        else if (slot == str_name)
        {
            // Return existing atom
            slot_filled = true;
        }
        else [[unlikely]]
        {
            // Slot occupied by a different string
            slot_filled = false;
        }
    }
    cache_lock->unlock();

    if (slot_filled) [[likely]]
    {
        return static_cast<int16_t>(atom);
    }
    else
    {
        // Ideally we would evict at this point, but that could introduce a race
        // condition if the pre-existing atom is being used on another thread
        return -1;
    }
}

StringName gdluau::string_name_for_atom(int p_atom)
{
    ERR_FAIL_COND_V_MSG(p_atom >= static_cast<int>(CACHE_SIZE), StringName(), "Invalid atom index.");

    if (p_atom < 0) [[unlikely]]
    {
        return StringName();
    }

    cache_lock->lock();
    // Taking a reference is safe because we don't evict entries once filled
    const StringName &str_name = cache[p_atom].str_name;
    cache_lock->unlock();

    return str_name;
}

CharString gdluau::char_string(const StringName &p_str_name)
{
    ERR_FAIL_COND_V_MSG(p_str_name.is_empty(), CharString(), "Cannot cache CharString for empty StringName.");

    unsigned atom = slot_for_string_name(p_str_name);
    bool different_string_in_slot = false;
    CharString char_str;

    // Try retrieving from cache
    cache_lock->lock();
    {
        CacheSlot &slot = cache[atom];
        if (slot.str_name.is_empty())
        {
            slot.str_name = p_str_name;
        }
        else if (slot.str_name == p_str_name) [[likely]]
        {
            // Cache hit
            char_str = slot.char_str;
        }
        else [[unlikely]]
        {
            different_string_in_slot = true;
        }
    }
    cache_lock->unlock();

    if (char_str.size() > 0) [[likely]]
    {
        return char_str;
    }

    // Cache miss: build CharString
    char_str = String(p_str_name).utf8();
    if (!different_string_in_slot) [[likely]]
    {
        // Populate cache
        cache_lock->lock();
        cache[atom].char_str = char_str;
        cache_lock->unlock();
    }

    return char_str;
}

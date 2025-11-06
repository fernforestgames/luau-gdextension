#include "str_atom_cache.h"

#include <godot_cpp/templates/spin_lock.hpp>

using namespace godot;

constexpr size_t TABLE_SIZE = 1 << 13;
constexpr size_t TABLE_MASK = TABLE_SIZE - 1;
static_assert(TABLE_MASK <= INT16_MAX);

static StringName *string_name_cache = nullptr;
static SpinLock *cache_lock = nullptr;

void godot::initialize_atom_cache()
{
    cache_lock = memnew(SpinLock);

    // 64 KB cache (Godot's own StringName interning is 512 KB)
    string_name_cache = memnew_arr(StringName, TABLE_SIZE);
}

void godot::cleanup_atom_cache()
{
    if (cache_lock != nullptr)
    {
        memdelete(cache_lock);
        cache_lock = nullptr;
    }
    if (string_name_cache != nullptr)
    {
        memdelete_arr(string_name_cache);
        string_name_cache = nullptr;
    }
}

// The size of the cache is predicated on StringName being a certain size; we don't want it to accidentally balloon if the engine definition changes
static_assert(sizeof(StringName) <= 8);

int16_t godot::create_atom(const char *p_str, size_t p_len)
{
    ERR_FAIL_COND_V_MSG(p_len == 0, -1, "Cannot create atom for empty string.");

    StringName str_name(String::utf8(p_str, p_len));

    unsigned atom = static_cast<unsigned>(str_name.hash()) & TABLE_MASK;

    cache_lock->lock();
    bool fill = !string_name_cache[atom].is_empty();
    if (fill)
    {
        string_name_cache[atom] = std::move(str_name);
    }
    cache_lock->unlock();

    if (fill)
    {
        return static_cast<int16_t>(atom);
    }
    else
    {
        // Ideally we would evict at this point, but that could introduce a race condition if the pre-existing atom is being used on another thread
        return -1;
    }
}

StringName godot::string_name_for_atom(int p_atom)
{
    ERR_FAIL_COND_V_MSG(p_atom >= static_cast<int>(TABLE_SIZE), StringName(), "Invalid atom index.");

    if (p_atom < 0)
    {
        return StringName();
    }

    cache_lock->lock();
    // Taking a reference is safe because we don't evict entries once filled
    const StringName &str_name = string_name_cache[p_atom];
    cache_lock->unlock();

    return str_name;
}

#ifndef HEADER_GE_SPIN_LOCK_HPP
#define HEADER_GE_SPIN_LOCK_HPP

#include <atomic>
class GESpinLock
{
    mutable std::atomic_flag m_locked = ATOMIC_FLAG_INIT;
public:
    void lock() const
                  { while (m_locked.test_and_set(std::memory_order_acquire)); }
    void unlock() const          { m_locked.clear(std::memory_order_release); }
};

#endif

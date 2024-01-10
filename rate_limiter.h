#pragma once

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <thread>

class RateLimiter {
    using Clock = std::chrono::high_resolution_clock;

   public:
    RateLimiter(int32_t rate) : max_permits_(rate), interval_(1000000000L / rate) {}

    void acquire() {
        auto now = Clock::now();
        if (now > next_free_) {
            stored_permits_ = std::min(max_permits_, stored_permits_ + (now - next_free_) / interval_);
            next_free_ = now;
        }

        auto wait = next_free_ - now;
        auto stored = std::min(static_cast<int64_t>(1), stored_permits_);
        auto fresh = 1 - stored;

        next_free_ += (fresh * interval_);
        stored_permits_ -= stored;

        if (wait > Clock::duration::zero()) {
            std::this_thread::sleep_for(wait);
        }
    }

   private:
    Clock::time_point next_free_{std::chrono::high_resolution_clock::now()};
    int64_t stored_permits_{0};
    const int64_t max_permits_;
    std::chrono::nanoseconds interval_;
};

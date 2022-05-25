#pragma once
#include <chrono>
#include <iostream>
#include <iomanip>

struct TimePoint {
    public:
    std::chrono::_V2::system_clock::time_point time;

    TimePoint() : time(std::chrono::system_clock::now()) {};
    TimePoint(std::chrono::_V2::system_clock::time_point t) : time(t) {};

    std::string toString(const std::string& format = "%Y/%m/%d %H:%M:%S ($P)") const;

    auto inline count() const { return time.time_since_epoch().count(); }

    auto inline diff(const TimePoint& t0) const {
        const auto t = time > t0.time ? time - t0.time : t0.time - time;
        return std::chrono::duration_cast<std::chrono::microseconds>(t).count();
    }

    auto inline diff() const { return diff(TimePoint()); }

    auto inline diff_seconds(const TimePoint& t0) const {
        return double(diff(t0))/1e6;
    }

    auto inline diff_seconds() const { return diff_seconds(TimePoint()); }
};

std::ostream& operator<<(std::ostream& s, const TimePoint& p);

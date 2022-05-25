#include "Clock.hpp"

std::string TimePoint::toString(const std::string& format) const {
    // https://stackoverflow.com/a/58523115
    std::time_t tt = std::chrono::system_clock::to_time_t(time);
    // std::tm tm = *std::gmtime(&tt); //GMT (UTC)
    std::tm tm = *std::localtime(&tt); //Locale time-zone, usually UTC by default.
    std::stringstream ss;
    ss << std::put_time( &tm, format.c_str() );
    auto str = ss.str();
    size_t start_pos = str.find("$P");
    if(start_pos != std::string::npos)
        str.replace(start_pos, 2, std::to_string(time.time_since_epoch().count()));
    return str;
}

std::ostream& operator<<(std::ostream& s, const TimePoint& p) {
    return s << p.toString();
}
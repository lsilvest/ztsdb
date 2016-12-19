// (C) 2016 Leonardo Silvestri
//
// This file is part of ztsdb.
//
// ztsdb is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ztsdb is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ztsdb.  If not, see <http://www.gnu.org/licenses/>.

#include "display.hpp"
#include "base_types.hpp"



/// convert to zstring.
template<>
arr::zstring arr::convert(const double& u) {
  return val::to_string(u, cfg::cfgmap);
}
template<>
arr::zstring arr::convert(const bool& u) {
  return val::to_string(u, cfg::cfgmap);
}
template<>
arr::zstring arr::convert(const Global::dtime& u) {
  return val::to_string(u, cfg::cfgmap);
}
template<>
arr::zstring arr::convert(const Global::duration& u) {
  return val::to_string(u, cfg::cfgmap);
}
template<>
arr::zstring arr::convert(const tz::period& u) {
  return val::to_string(u, cfg::cfgmap);
}
template<>
arr::zstring arr::convert(const tz::interval& u) {
  return val::to_string(u, cfg::cfgmap);
}


/// convert to string.
template<>
std::string arr::convert(const double& u) {
  return val::to_string(u, cfg::cfgmap);
}
template<>
std::string arr::convert(const bool& u) {
  return val::to_string(u, cfg::cfgmap);
}
template<>
std::string arr::convert(const Global::dtime& u) {
  return val::to_string(u, cfg::cfgmap);
}
template<>
std::string arr::convert(const Global::duration& u) {
  return val::to_string(u, cfg::cfgmap);
}
template<>
std::string arr::convert(const tz::period& u) {
  return val::to_string(u, cfg::cfgmap);
}
template<>
std::string arr::convert(const tz::interval& u) {
  return val::to_string(u, cfg::cfgmap);
}


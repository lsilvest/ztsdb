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


#include <sys/stat.h>
#include <system_error>
#include <exception>
#include <iostream>
#include <sstream>
#include <limits>
#include <algorithm>
#include "zone.hpp"


using namespace std::chrono;
using namespace std::literals;


// binary search.
static size_t bs(time_t t, time_t* ats, int n) {
  size_t first = 0;
  size_t last = n - 1;
  size_t middle = (first+last)/2;
 
  while (first <= last) {
    if (ats[middle] < t) {
      first = middle + 1;    
    }
    else if (ats[middle] == t) {
      break;
    }
    else {
      last = middle - 1;
    }
    middle = (first + last)/2;
  }
  return middle;
}


void tz::Zone::buildRl() {             // build the reverse lookup map
  auto prev = s->defaulttype;
  rl[std::numeric_limits<time_t>::min()] = {prev};
  if (!s->timecnt) {
    // since there are no transitions, set the offset for all
    // representable times:
    rl[std::numeric_limits<time_t>::max()] = {prev};
  }
  else {
    for (int i=0; i<s->timecnt; i++) {
      auto cur = s->types[i];
      time_t offsetdiff = s->ttis[cur].tt_gmtoff - s->ttis[prev].tt_gmtoff;
      if (offsetdiff > 0) {
        // the time jumped forwards, so we have unrepresentable times:
        //std::cout << "adding unrepresentable" << std::endl;
        rl[s->ats[i] + s->ttis[prev].tt_gmtoff] = {};
        rl[s->ats[i] + s->ttis[cur].tt_gmtoff] = {cur};
      } else if (offsetdiff < 0) {
        // the time went backwards, so we have ambiguous times:
        rl[s->ats[i] + s->ttis[cur].tt_gmtoff] = {prev, cur};
        rl[s->ats[i] + s->ttis[prev].tt_gmtoff] = {cur};
        //std::cout << "adding ambiguous" << std::endl;
      }
      else {
        // else it's the same offset (but maybe the abbrev changed?), so
        // just one unambiguous entry:
        rl[s->ats[i] + s->ttis[cur].tt_gmtoff] = {cur};
      }

      prev = cur;
    }
  }
}


tz::Zone::Zone(fsys::path p) : s(std::make_unique<tz::state>()) {
  struct stat st = {0};
  if (stat(p.string().c_str(), &st) != 0) {
    throw std::system_error(std::error_code(errno, std::system_category()), 
                            "cannot stat " + p.string());
  }
  
  if (tzload(p.string().c_str(), s.get(), 0) != 0) {
    throw std::range_error("cannot load tz " + p.string());
  }

  buildRl();                    // build the reverse lookup map
}


std::pair<Global::dtime::duration, const char*> tz::Zone::getoffset(Global::dtime dt) const {
  const int64_t den = Global::dtime::duration::period::den;
  const time_t d = dt.time_since_epoch().count() / den;

  if (s->timecnt && s->ats[0] < d) {
    auto i = bs(d, s->ats, s->timecnt);
    
    return make_pair(s->ttis[s->types[i]].tt_gmtoff * 1s, 
                     &s->chars[s->ttis[s->types[i]].tt_abbrind]);
  }
  else {
    return make_pair(s->ttis[s->defaulttype].tt_gmtoff * 1s, 
                     &s->chars[s->ttis[s->defaulttype].tt_abbrind]);
  }
}


Global::dtime::duration tz::Zone::getoffset(Global::dtime dt, int& pos) const {
  const int64_t den = Global::dtime::duration::period::den;
  const time_t d = dt.time_since_epoch().count() / den;
  if (s->timecnt && d > s->ats[0]) {
    pos = bs(d, s->ats, s->timecnt);
    return s->ttis[s->types[pos]].tt_gmtoff * 1s;
  }
  else {
    pos = -1;
    return s->ttis[s->defaulttype].tt_gmtoff * 1s;
  }
}


Global::dtime::duration tz::Zone::getoffset(Global::dtime dt, int& pos, int dir) const {
  if (pos >= 0) {
    auto sign = std::copysign(1, dir);
    if (sign >= 0) {
      while (pos < s->timecnt - 1 && dt.time_since_epoch() > s->ats[pos+1] * 1s) {
        ++pos;
      }
      return s->ttis[s->types[pos]].tt_gmtoff * 1s;
    }
    else {                      // sign < 0
      while (pos > 0 && dt.time_since_epoch() < s->ats[pos] * 1s) {
        --pos;
      }
      if (dt.time_since_epoch() > s->ats[pos] * 1s)
        return s->ttis[s->types[pos]].tt_gmtoff * 1s;
      else {
        return s->ttis[s->defaulttype].tt_gmtoff * 1s;
      }
    }
  }
  else {
    return getoffset(dt, pos);
  }
}


std::set<Global::dtime::duration> tz::Zone::getReverseOffset(Global::dtime dt) const {
  const int64_t den = Global::dtime::duration::period::den;
  const time_t d = dt.time_since_epoch().count() / den;

  auto elt = rl.upper_bound(d); // upper_bound because we will decrement
  --elt;                        // can always be decremented, because
                                // the first element of rl is always
                                // the lower representable date
  auto typeset = elt->second;
  std::set<Global::dtime::duration> res;
  for (auto elt : typeset) {
    using namespace std::chrono;
    using namespace std::literals;
    res.insert(s->ttis[elt].tt_gmtoff * 1s);
  }
    return res;
}

tz::Zones::Zones() { }


tz::Zones::Zones(fsys::path p) {
  init(p);
}


void tz::Zones::init(fsys::path p) {
  // go through all the zones in zoneinfo
  // pick up all abbrevs and build table of them
  // build hash table of all zones
  //
  if (!is_directory(p)) {
    throw std::range_error("can't find directory " + p.string());
  }
  if (p.string().back() != '/') {        // portability! LLL
    p += '/';
  }

  // Note from the Boost manual: 
  //
  // By default, recursive_directory_iterator does not follow
  // directory symlinks. To follow directory symlinks, specify opt as
  // symlink_option::recurse

  fsys::recursive_directory_iterator dir(p), end;
  
  const auto len = p.string().length();
  while (dir != end) {
    if (dir->path().filename() == "posix" || dir->path().filename() == "right") {
      dir.no_push();
    }
    if (!is_directory(*dir) 
        && !(dir->path().filename().extension() == ".tab")
        && !(dir->path().filename().extension() == ".list")) {
      //      std::cout << "Zone: " << dir->path().string().substr(len) << ":" << std::endl;
      const auto& elt = m.emplace(make_pair(dir->path().string().substr(len), Zone(*dir)));
      //      std::cout << tz::to_string(elt.first->second) << std::endl;
      if (!elt.second) {
        throw std::range_error("can't emplace zone "s + dir->path().c_str());
      }
      // const auto s = elt.first->second.s.get(); // ptr to the zone state information

// the following doesn't work: same abbrevs have different offsets at
// different times. if the info is the IANA db is sound (or rather if
// reality was coherent), then it should be possible to build the
// lookup from an abbrev; try it when we get some time LLL
//
//       // go through all abbreviations and put them in a table:
//       for (int i=0; i<s->typecnt; i++) {
//         // std::cout << "inserting: " << &s->chars[s->ttis[i].tt_abbrind] << std::endl;
//         if (&s->chars[s->ttis[i].tt_abbrind] != std::string("LMT")) {
//           // local mean time is a weird abbreviation and doesn't always map to the same offset
//           // so we ignore it...
//           const auto elt = a.emplace(std::make_pair(&s->chars[s->ttis[i].tt_abbrind],
//                                      s->ttis[i].tt_gmtoff));
//           if (!elt.second) {
//             // std::cout << "a.emplace failed for " << &s->chars[s->ttis[i].tt_abbrind] << std::endl;
//             // check that we don't have an abbreviation with two different offsets:
//             if (a[&s->chars[s->ttis[i].tt_abbrind]] != s->ttis[i].tt_gmtoff) {
// //              throw std::range_error(std::string("tz::Zones::init: differing offsets for abbreviation ") +
// //                                     &s->chars[s->ttis[i].tt_abbrind]);
//               std::cout << "tz::Zones::init: differing offsets for abbreviation "
//                         << &s->chars[s->ttis[i].tt_abbrind] << " in file" << dir->path().filename()
//                         << std::endl;
//             }
//           }
//         }
//       }
    }
    ++dir;
  }
  // std::cout << "all loaded" << std::endl;
}


std::string tz::to_string(const Zone& z) {
  std::stringstream ss;
  ss << "leapcnt "      << z.s->leapcnt     << std::endl;
  ss << "timecnt "      << z.s->timecnt     << std::endl;
  ss << "typecnt "      << z.s->typecnt     << std::endl;
  ss << "charcnt "      << z.s->charcnt     << std::endl;
  ss << "goback "       << z.s->goback      << std::endl;
  ss << "goahead "      << z.s->goahead     << std::endl;
  ss << "defaulttype "  << z.s->defaulttype << std::endl;
  ss << "types: " << std::endl;
  for (int i=0; i<z.s->typecnt; i++) {
    ss << z.s->ttis[i].tt_gmtoff << ", " 
              << z.s->ttis[i].tt_isdst << ", "
              << &z.s->chars[z.s->ttis[i].tt_abbrind] << ", "
              << z.s->ttis[i].tt_ttisstd << ", " 
              << z.s->ttis[i].tt_ttisgmt
              << std::endl;
  }
  ss << "transitions: " << std::endl;
  for (int i=0; i<z.s->timecnt; i++) {
    ss << z.s->ats[i] << " , " << int(z.s->types[i]) << std::endl;
  }

  return ss.str();
}


std::string tz::summary(const Zone& z) {
  std::stringstream ss;
  ss << "timecnt "  << z.s->timecnt << ", typecnt "  << z.s->typecnt << std::endl;
  return ss.str();
}


const tz::Zone& tz::Zones::find(const std::string& s) const {
  auto r = m.find(s);
  if (r != m.end()) {
    return r->second;
  }
  else {
    throw std::range_error("can't find timezone " + s);
  }
}


std::string tz::to_string(const Zones& zs) {
  std::stringstream ss;
  for (auto i = zs.m.cbegin(); i != zs.m.cend(); ++i) {
    ss << i->first << ": " << summary(i->second); 
  }

  return ss.str();
}

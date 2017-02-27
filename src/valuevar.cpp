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


#include <arpa/inet.h>
#include "valuevar.hpp"
#include "env.hpp"


// #define DEBUG


val::VClos::VClos(const Function* f_a) { 
  f = std::shared_ptr<Function>(f_a->clone()); // clone: need a full copy of the parse tree
  f->processFormlist(argMap, ellipsisPos);
}


val::VConn::VConn(const string &ip_p, int port_p, Global::conn_id_t id_p) : 
  ip(ip_p), port(port_p), id(id_p) { }


void val::VConn::setId(Global::conn_id_t id_p) {
  id = id_p;
}


val::VTimer::VTimer(const uint64_t nanosecs_p, 
                    shared_ptr<E> loop_p, 
                    shared_ptr<E> once_p,
                    size_t loop_max_p) : 
  nanosecs(nanosecs_p), loop(loop_p), once(once_p), loop_max(loop_max_p), 
  loop_n(0), loop_failed_n(0), done_first(false) 
{
  fd = timerfd_create(CLOCK_MONOTONIC, 0);
  if (fd == -1) {
    throw std::system_error(std::error_code(errno, std::system_category()), "timerfd_create");
  }
}


void val::VTimer::start() {
  itimerspec tmr;
  bzero(&tmr, sizeof(tmr));
  tmr.it_value.tv_sec = nanosecs / 1000000000;
  tmr.it_value.tv_nsec = nanosecs % 1000000000;
  tmr.it_interval.tv_sec = tmr.it_value.tv_sec;
  tmr.it_interval.tv_nsec = tmr.it_value.tv_nsec;
  if (timerfd_settime(fd, 0, &tmr, NULL) == -1) {
    throw std::system_error(std::error_code(errno, std::system_category()), 
                            "VTimer::start: timerfd_settime");
  }
}


void val::VTimer::stop() {
  itimerspec tmr;
  bzero(&tmr, sizeof(tmr));
  if (timerfd_settime(fd, 0, &tmr, NULL) == -1) {
    throw std::system_error(std::error_code(errno, std::system_category()), 
                            "VTimer::stop: timerfd_settime");
  }
}


val::VTimer::~VTimer() { stop(); }


void val::VFuture::setvalptr(val::Value& val_p, const interp::shpfrm& frame_p) {
  val = &val_p;
  frame = frame_p;
}


val::Value* val::VFuture::getvalptr() {
  if (auto f = frame.lock()) {
    return val;
  }
  else {
    return nullptr;
  }
}


std::string val::VFuture::to_string() const {
  stringstream ss;
  if (auto f = frame.lock()) {
    ss << f.get();
  }
  else {
    ss << f;
  }
  return ss.str();
}



val::VList::VList() : a(arr::rsv, {0}) { }
val::VList::VList(const Array<Value>& a_p) : a(a_p) { }
val::VList::VList(const VList& l) : a(l.a) { }
val::VList::VList(const vector<pair<string, Value>>& l_p, bool concat) : a(rsv, {0}) 
{
  for (auto& p : l_p) {
    if (!concat || p.second.which() != vt_list) {
      a.concat(p.second, p.first);
    } else {
      auto& alist = get<SpVList>(p.second);
      for (size_t i=0; i<alist->size(); i++) {
        const auto& e = alist->a[i]; 
        const auto& ename = alist->a.getnames(0)[i];
        auto aname = p.first=="" ?
          ename : p.first + (ename=="" ? std::to_string(i+1) : '.' + ename); 
        a.concat(e, aname);
      }
    }
  }
}


// to reduce boilerplate switch code:
template <template<typename...> class F, typename... T>
static inline void apply_to_types(val::Value& v) { }

template <template<typename...> class F, val::ValType H, val::ValType... T>
static inline void apply_to_types(val::Value& v) {
  H == v.which() ? F<typename val::gettype<H>::TP>::f(v) : apply_to_types<F, T...>(v);
}


template<typename T>
struct setTmp_helper {
  static void f(val::Value& v) {
    auto& a = get<T>(v);
    a.setTmp();
  }
};

void val::setTmp(val::Value& v) {
  apply_to_types<setTmp_helper, 
                 val::vt_double, 
                 val::vt_bool, 
                 val::vt_time, 
                 val::vt_duration, 
                 val::vt_interval, 
                 val::vt_period, 
                 val::vt_string, 
                 val::vt_zts,
                 val::vt_list>(v);
}


template<typename T>
struct resetTmp_helper {
  static void f(val::Value& v) {
    auto& a = get<T>(v);
    a.resetTmp();
  }
};

void val::resetTmp(val::Value& v) {
  apply_to_types<resetTmp_helper, 
                 val::vt_double, 
                 val::vt_bool, 
                 val::vt_time, 
                 val::vt_duration, 
                 val::vt_interval, 
                 val::vt_period, 
                 val::vt_string, 
                 val::vt_zts,
                 val::vt_list>(v);
}


template<typename T>
struct setConst_helper {
  static void f(val::Value& v) {
    auto& a = get<T>(v);
    a.setConst();
  }
};

void val::setConst(val::Value& v) {
  apply_to_types<setConst_helper, 
                 val::vt_double, 
                 val::vt_bool, 
                 val::vt_time, 
                 val::vt_duration, 
                 val::vt_interval, 
                 val::vt_period, 
                 val::vt_string, 
                 val::vt_zts,
                 val::vt_list>(v);
}


template<typename T>
struct setLast_helper {
  static void f(val::Value& v) {
    auto& a = get<T>(v);
    a.setLast();
  }
};

void val::setLast(val::Value& v) {
  apply_to_types<setLast_helper, 
                 val::vt_double, 
                 val::vt_bool, 
                 val::vt_time, 
                 val::vt_duration, 
                 val::vt_interval, 
                 val::vt_period, 
                 val::vt_string, 
                 val::vt_zts,
                 val::vt_list>(v);
}


template<typename T>
struct setRef_helper {
  static void f(val::Value& v) {
    auto& a = get<T>(v);
    a.setRef();
  }
};

void val::setRef(val::Value& v) {
  apply_to_types<setRef_helper, 
                 val::vt_double, 
                 val::vt_bool, 
                 val::vt_time, 
                 val::vt_duration, 
                 val::vt_interval, 
                 val::vt_period, 
                 val::vt_string, 
                 val::vt_zts,
                 val::vt_list>(v);
}


template<typename T>
struct resetRef_helper {
  static void f(val::Value& v) {
    auto& a = get<T>(v);
    a.resetRef();
  }
};

void val::resetRef(val::Value& v) {
  apply_to_types<resetRef_helper, 
                 val::vt_double, 
                 val::vt_bool, 
                 val::vt_time, 
                 val::vt_duration, 
                 val::vt_interval, 
                 val::vt_period, 
                 val::vt_string, 
                 val::vt_zts,
                 val::vt_list>(v);
}


// to reduce boilerplate switch code:
template <template<typename...> class F, typename... T>
static inline bool apply_to_types_bool(const val::Value& v) { return false; }

template <template<typename...> class F, val::ValType H, val::ValType... T>
static inline bool apply_to_types_bool(const val::Value& v) {
  return H == v.which() ? F<typename val::gettype<H>::TP>::f(v) : apply_to_types_bool<F, T...>(v);
}

template<typename T>
struct isLocked_helper {
  static bool f(const val::Value& v) {
    auto& a = get<T>(v);
    return a.isLocked();
  }
};


bool val::isLocked(const val::Value& v) { 
  return apply_to_types_bool<isLocked_helper, 
                             val::vt_double, 
                             val::vt_bool, 
                             val::vt_time, 
                             val::vt_duration, 
                             val::vt_interval, 
                             val::vt_period, 
                             val::vt_string, 
                             val::vt_zts,
                             val::vt_list>(v);
}


template<typename T>
struct isRef_helper {
  static bool f(const val::Value& v) {
    auto& a = get<T>(v);
    return a.isRef();
  }
};


bool val::isRef(const val::Value& v) { 
  return apply_to_types_bool<isRef_helper, 
                             val::vt_double, 
                             val::vt_bool, 
                             val::vt_time, 
                             val::vt_duration, 
                             val::vt_interval, 
                             val::vt_period, 
                             val::vt_string, 
                             val::vt_zts,
                             val::vt_list>(v);
}


template<typename T>
struct isTmp_helper {
  static bool f(const val::Value& v) {
    auto& a = get<T>(v);
    return a.isTmp();
  }
};


bool val::isTmp(const val::Value& v) { 
  return apply_to_types_bool<isTmp_helper, 
                             val::vt_double, 
                             val::vt_bool, 
                             val::vt_time, 
                             val::vt_duration, 
                             val::vt_interval, 
                             val::vt_period, 
                             val::vt_string, 
                             val::vt_zts,
                             val::vt_list>(v);
}


template<typename T>
struct isConst_helper {
  static bool f(const val::Value& v) {
    auto& a = get<T>(v);
    return a.isConst();
  }
};


bool val::isConst(const val::Value& v) { 
  return apply_to_types_bool<isConst_helper, 
                             val::vt_double, 
                             val::vt_bool, 
                             val::vt_time, 
                             val::vt_duration, 
                             val::vt_interval, 
                             val::vt_period, 
                             val::vt_string, 
                             val::vt_zts,
                             val::vt_list>(v);
}


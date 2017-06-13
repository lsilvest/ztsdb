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


#include <ctime>
#include <cmath>
#include "display.hpp"
#include "double.hpp"
#include "timezone/ztime.hpp"
#include "timezone/zone.hpp"


extern tz::Zones tzones;


  // ‘digits’: controls the number of digits to print when printing
  //       numeric values.  It is a suggestion only.  Valid values are
  //       1...22 with default 7.  See the note in ‘print.default’ about
  //       values greater than 15.


  //  ‘digits.secs’: controls the maximum number of digits to print when
  //       formatting time values in seconds.  Valid values are 0...6
  //       with default 0.  See ‘strftime’.

  //  ‘width’: controls the maximum number of columns on a line used in
  //       printing vectors, matrices and arrays, and when filling by
  //       ‘cat’.

  //       Columns are normally the same as characters except in East
  //       Asian languages.

  //       You may want to change this if you re-size the window that R
  //       is running in.  Valid values are 10...10000 with default
  //       normally 80.  (The limits on valid values are in file
  //       ‘Print.h’ and can be changed by re-compiling R.)  Some R
  //       consoles automatically change the value when they are
  //       resized.

  //  ‘scipen’: integer.  A penalty to be applied when deciding to print
  //       numeric values in fixed or exponential notation.  Positive
  //       values bias towards fixed and negative towards scientific
  //       notation: fixed notation will be preferred unless it is more
  //       than ‘scipen’ digits wider.

  // Large number of digits:

  //      Note that for large values of ‘digits’, currently for ‘digits >=
  //      16’, the calculation of the number of significant digits will
  //      depend on the platform's internal (C library) implementation of
  //      ‘sprintf()’ functionality.

  // > cbind(matrix(c(1.23432e200, 1.2, 3e6), 3, 1), c(1,2,3))
  //                              [,1]  [,2]
  // [1,] 1.234320000000000010204e+200 1e+00
  // [2,]  1.199999999999999955591e+00 2e+00
  // [3,]  3.000000000000000000000e+06 3e+00
  // > options(scipen=6)
  // > cbind(matrix(c(1.23432e200, 1.2, 3e6), 3, 1), c(1,2,3))
  //                              [,1] [,2]
  // [1,] 1.234320000000000010204e+200    1
  // [2,]  1.199999999999999955591e+00    2
  // [3,]  3.000000000000000000000e+06    3

  // > cbind(matrix(c(1.23432e200, 1.2, 3e6), 3, 1), c(1,2,3))
  //              [,1] [,2]
  // [1,] 1.23432e+200 1   
  // [2,] 1.2          2   
  // [3,] 3e+6         3   
  // > 

// ------ double

Vector<zstring> doubleToStringFixedScientific(Vector<double> v, int digits) { 
  Vector<zstring> vs;
  for (auto e : v) {
    vs.push_back(ztsdb::fixed_scientific(e, digits));
  }
  return vs;
}

Vector<zstring> doubleToStringFixedDecimal(Vector<double> v, int digits) { 
  Vector<zstring> vs;
  for (auto e : v) {
    vs.push_back(ztsdb::fixed_decimal(e, digits));
  }
  return vs;
}

static Vector<zstring> doubleToStringShortestScientific(Vector<double> v) { 
  Vector<zstring> vs;
  for (auto e : v) {
    vs.push_back(ztsdb::shortest_scientific(e));
  }
  return vs;
}

static Vector<zstring> doubleToStringShortestDecimal(Vector<double> v) { 
  Vector<zstring> vs;
  for (auto e : v) {
    vs.push_back(ztsdb::shortest_decimal(e));
  }
  return vs;
}


static bool anyScientific(const Vector<zstring>& v) {
  return std::any_of(v.begin(), v.end(), [](zstring s) { return s.find('e') < s.size(); });
}


static bool requireScientific(const Vector<zstring>& sci, const Vector<zstring>& dec, int scipen) {
  // R says:
  //  ‘scipen’: integer.  A penalty to be applied when deciding to print
  //       numeric values in fixed or exponential notation.  Positive
  //       values bias towards fixed and negative towards scientific
  //       notation: fixed notation will be preferred unless it is more
  //       than ‘scipen’ digits wider.
  Vector<ssize_t> sciMinusDec;
  transform(sci.begin(), sci.end(), dec.begin(), back_inserter(sciMinusDec),
            [](const zstring& sci_str, const zstring& dec_str) {
              return dec_str.size() - sci_str.size();
            });
  return std::any_of(sciMinusDec.begin(), sciMinusDec.end(), 
                     [scipen](ssize_t diff) { return diff > scipen; });
}


static bool anyNegative(const Vector<zstring>& v) {
  return std::any_of(v.begin(), v.end(), 
                     [](zstring s) { if (s[0] == '-') return true; else return false; });
}

static Vector<zstring> paddNonNegative(const Vector<zstring>& v) {
  Vector<zstring> vs;
  for (const auto& e : v) {
    vs.push_back(e[0] == '-' ? e : " " + e);
  }
  return vs;
}

static Vector<zstring> alignDecimal(const Vector<zstring>& v) {
  // Align on floating point decimal
  Vector<size_t> decimalPos;
  transform(v.begin(), v.end(), back_inserter(decimalPos), 
            [](const zstring& s) { return s.find('.'); });
  size_t maxDecimalPos = *std::max_element(decimalPos.begin(), decimalPos.end());
  Vector<zstring> vs;
  for (size_t i=0; i<v.size(); ++i) {
    vs.push_back(zstring(' ', maxDecimalPos - decimalPos[i]) + v[i]);
  }
  return vs;
}


// we can never have more than 17 floating point digits
// if we have 'digits' < 17 we truncate, like in R
struct DoubleToString {
  DoubleToString(const cfg::CfgMap& cfg_p) : cfg(cfg_p) { }

  Vector<zstring> operator()(Vector<double> v) { 
    auto digits = get<int64_t>(cfg.get("digits"s));

    auto fixedScientific =    doubleToStringFixedScientific(v, digits);
    auto fixedDecimal =       doubleToStringFixedDecimal(v, digits);
    auto shortestScientific = doubleToStringShortestScientific(v);
    auto shortestDecimal  =   doubleToStringShortestDecimal(v);

    // combine to make a "best decimal" and "best scientific" (i.e. take shortest of fixed/shortest)
    Vector<zstring> decimal;
    std::transform(shortestDecimal.begin(), shortestDecimal.end(), fixedDecimal.begin(), 
                   std::back_inserter(decimal), 
                   [](zstring& a, zstring&b) { 
                     // it's bad that 'ToDecimal' silently
                     // returns "" when it has too many digits...
                     // it also returns 1e50 as 
                     // 100000000000000007629769841091887003294964970946560.00000
                     // rather than 100000000000000000000000000000000000000000000000000.00000
                     // LLL
                     if (b.size() == 0) return a;
                     else return a.size() < b.size() ? a : b; });
    Vector<zstring> scientific;
    std::transform(shortestScientific.begin(), shortestScientific.end(), fixedScientific.begin(), 
                   std::back_inserter(scientific), 
                   [](zstring& a, zstring&b) { return a.size() < b.size() ? a : b; });


    auto scipen = get<int64_t>(cfg.get("scipen"s));
    if (anyScientific(decimal) || requireScientific(scientific, decimal, scipen)) {
      // take out e+0 LLL
      return anyNegative(scientific) ? paddNonNegative(scientific) : scientific;
    }
    else {
      return alignDecimal(decimal);
    }

    return scientific;
  }

  const cfg::CfgMap& cfg;
};

template<>
Array<zstring> val::arrayToString(const Array<double>& a, const cfg::CfgMap& cfg) {
  return Array<zstring>(a, DoubleToString(cfg), true);
}

template<>
Vector<zstring> val::vectorToString(const Vector<double>& v, const cfg::CfgMap& cfg) {
  return DoubleToString(cfg)(v);
}


// ------ Global::dtime

static bool anyFractionalSecond(const Vector<Global::dtime>& v) {
  return std::any_of(v.begin(), v.end(), [](Global::dtime d) { 
      return d.time_since_epoch().count() % 1000000000 != 0; 
    });
}

struct DTimeToString {
  DTimeToString(const cfg::CfgMap& cfg_p) : cfg(cfg_p) { }
  Vector<zstring> operator()(Vector<Global::dtime> v) {
    auto frac = anyFractionalSecond(v);
    auto tz = get<std::string>(cfg.get("timezone"s));
    Vector<zstring> vs;
    std::transform(v.begin(), v.end(), 
                   std::back_inserter(vs), 
                   [tz, frac](const Global::dtime& d) { 
                     return tz::to_string(d, "", tzones.find(tz), tz, true, frac); });
    return vs;
  }
  const cfg::CfgMap& cfg;
};

template<>
Array<zstring> val::arrayToString(const Array<Global::dtime>& a, const cfg::CfgMap& cfg) {
  return Array<zstring>(a, DTimeToString(cfg), true);
}

template<>
Vector<zstring> val::vectorToString(const Vector<Global::dtime>& v, const cfg::CfgMap& cfg) {
  return DTimeToString(cfg)(v);
}


// ------ tz::interval

static bool anyFractionalSecond(const Vector<tz::interval>& v) {
  return std::any_of(v.begin(), v.end(), [](tz::interval i) { 
      return 
        i.s.time_since_epoch().count() % 1000000000 != 0 ||
        i.e.time_since_epoch().count() % 1000000000 != 0 ; 
    });
}

struct IntervalToString {
  IntervalToString(const cfg::CfgMap& cfg_p) : cfg(cfg_p) { }
  Vector<zstring> operator()(Vector<tz::interval> v) {
    auto frac = anyFractionalSecond(v);
    auto timezone = get<std::string>(cfg.get("timezone"s));
    Vector<zstring> vs;
    std::transform(v.begin(), v.end(), 
                   std::back_inserter(vs), 
                   [timezone, frac](const tz::interval& i) { 
                     return tz::to_string(i, "", tzones.find(timezone), timezone, true, frac); });
    return vs;
  }
  const cfg::CfgMap& cfg;
};

template<>
Array<zstring> val::arrayToString(const Array<tz::interval>& a, const cfg::CfgMap& cfg) {
  return Array<zstring>(a, IntervalToString(cfg), true);
}

template<>
Vector<zstring> val::vectorToString(const Vector<tz::interval>& v, const cfg::CfgMap& cfg) {
  return IntervalToString(cfg)(v);
}

// ------ tz::period

struct PeriodToString {
  PeriodToString(const cfg::CfgMap& cfg_p) : cfg(cfg_p) { }
  Vector<zstring> operator()(Vector<tz::period> v) {
    Vector<zstring> vs;
    std::transform(v.begin(), v.end(), 
                   std::back_inserter(vs), 
                   [](const tz::period& i) { 
                     return tz::to_string(i); });
    return vs;
  }
  const cfg::CfgMap& cfg;
};

template<>
Array<zstring> val::arrayToString(const Array<tz::period>& a, const cfg::CfgMap& cfg) {
  return Array<zstring>(a, PeriodToString(cfg), true);
}

template<>
Vector<zstring> val::vectorToString(const Vector<tz::period>& v, const cfg::CfgMap& cfg) {
  return PeriodToString(cfg)(v);
}


static string display_static(const val::Value& v, const cfg::CfgMap& cfg, size_t& left) {
  stringstream ss;

  switch (v.which()) {
  case val::vt_list: {
    const auto& l = get<const val::SpVList>(v);
    ss << val::display(l, cfg, "", left);
    break;
  }
  case val::vt_std_string:
    ss << get<const std::string>(v);
    break;
  case val::vt_null:
  case val::vt_code:
  case val::vt_clos:
  case val::vt_connection:
  case val::vt_timer:
  case val::vt_builting:
  case val::vt_error:
  case val::vt_std_int:
    ss << to_string(v, cfg);
    --left;
    break;
  case val::vt_string: {
    const auto& a = *get<const val::SpVAS>(v);
    ss << val::display(a, a.getnames(0).names, cfg, left);
    break;
  }
  case val::vt_double:  {
    const auto& a = *get<const val::SpVAD>(v);
    ss << val::display(a, a.getnames(0).names, cfg, left);
    break;
  }
  case val::vt_bool:  {
    const auto& a = *get<const val::SpVAB>(v);
    ss << val::display(a, a.getnames(0).names, cfg, left);
    break;
  }
  case val::vt_time:  {
    const auto& a = *get<const val::SpVADT>(v);
    ss << val::display(a, a.getnames(0).names, cfg, left);
    break;
  }
  case val::vt_duration:  {
    const auto& a = *get<const val::SpVADUR>(v);
    ss << val::display(a, a.getnames(0).names, cfg, left);
    break;
  }
  case val::vt_interval:  {
    const auto& a = *get<const val::SpVAIVL>(v);
    ss << val::display(a, a.getnames(0).names, cfg, left);
    break;
  }
  case val::vt_period:  {
    const auto& a = *get<const val::SpVAPRD>(v);
    ss << val::display(a, a.getnames(0).names, cfg, left);
    break;
  }
  case val::vt_zts:  {
    const auto& a = *get<const val::SpZts>(v);
    ss << val::display(a.getArray(), a.getIndex().getcol(0), cfg, left);
    break;
  }
  case val::vt_named: {
    const auto& a = get<const val::VNamed>(v);
    ss << "[" << val::display(a.name) << "; " << val::display(a.val) << "]";
    --left;
    break;
  }
  case val::vt_ptr: {
    const auto& a = get<const val::VPtr>(v);
    ss << (val::isRef(v) ? "ref: " : "not_ref: ") << a.p ? val::display(*a.p) : "vptr(null)";
    --left;
    break;
  }
  case val::vt_future: {
    const auto& f = get<const val::SpFuture>(v);
    ss << "Future(" << f->to_string() << ")";
    --left;
    break;
  }
  default:
    throw std::domain_error("val::display: type unimplemented " + std::to_string(v.which()));
  }
  return ss.str();
}


string val::display(const val::Value& v) {
  size_t left = static_cast<arr::idx_type>(get<int64_t>(cfg::cfgmap.get("max.print")));
  return display_static(v, cfg::cfgmap, left);
}


string val::display(const val::SpVList& l,
                    const cfg::CfgMap& cfg,
                    string prefix,
                    size_t& left)
{
  stringstream ss;
  if (!l->size()) {
    return "list()";            // empty list
  }
  size_t j=0;
  for (; j<l->size(); ++j) {
    auto curPrefix = prefix +
      ((*l->a.names[0])[j] == "" ? "[[" + std::to_string(j+1) + "]]" : "$" + (*l->a.names[0])[j]);
    ss << curPrefix << endl;
    if (!left) break;           // break just after the header if necessary
    if (l->a[j].which() == val::vt_list) {
      auto& subl = get<SpVList>(l->a[j]);
      ss << display(subl, cfg, curPrefix, left) << endl; // recursive call!
    } else {
      ss << display_static(l->a[j], cfg, --left) << endl;
    }
    ss << endl;
  }
  if (j < l->size()) {
    ss << " [ reached getOption(""max.print"") -- omitted "
       << l->size() - j << " list entries ]" << endl;
  }
  return ss.str().substr(0, ss.str().length()-1); // pull out trailing line break
}


string val::to_string(const arr::zts& ts, const cfg::CfgMap& cfg) {
  size_t left = static_cast<arr::idx_type>(get<int64_t>(cfg.get("max.print")));
  return display(ts.getArray(), ts.getIndex().getcol(0), cfg, left); 
}
string val::to_string(const val::SpZts& ts, const cfg::CfgMap& cfg) {
  size_t left = static_cast<arr::idx_type>(get<int64_t>(cfg.get("max.print")));
  return display(ts->getArray(), ts->getIndex().getcol(0), cfg, left);
}
string val::to_string(const val::VCode& v, const cfg::CfgMap& cfg) { 
  return "expression(" + to_string(*v.expr) + ')'; 
}
string val::to_string(const shared_ptr<val::VClos>& v, const cfg::CfgMap& cfg) { 
  return to_string(*v->f); 
}
string val::to_string(const val::VClos& v, const cfg::CfgMap& cfg) { 
  return to_string(*v.f); 
}
string val::to_string(const val::VNull& v, const cfg::CfgMap& cfg) { 
  return "NULL"; 
}
string val::to_string(const std::shared_ptr<val::VBuiltinG>& v, const cfg::CfgMap& cfg) { 
  return "native: " + to_string(*v->signature); 
}
string val::to_string(const val::SpFuture& v, const cfg::CfgMap& cfg) { 
  return v->to_string();  
}
string val::to_string(const val::VConn& v, const cfg::CfgMap& cfg) { 
  return v.ip + ":" + std::to_string(v.port) + " [" + std::to_string(v.id) + ']';
}
string val::to_string(const val::SpTimer& v, const cfg::CfgMap& cfg) { 
  return std::to_string(v->fd) + " : " + std::to_string(v->nanosecs) + 
    " \nloop:\n"  + to_string(*v->loop) + "\nonce:\n"  + to_string(*v->once);
}
// this one is a quick and dirty display for debugging:
string val::to_string(const VList& v, const cfg::CfgMap& cfg) {
  stringstream ss;
  ss << "list(";
  for (size_t i = 0; i<v.size(); ++i) {
    if ((*v.a.names[0])[i] != "") {
      ss << (*v.a.names[0])[i] << '=';
    }
    ss << apply_visitor(to_string_v(), v.a[i], cfg);
    if (i < v.size() - 1) { ss << ", "; } 
  } 
  ss << ")";
  return ss.str();
}
string val::to_string(const SpVList& l, const cfg::CfgMap& cfg) {
  return val::to_string(*l, cfg);
}
string val::to_string(const val::VNamed& v, const cfg::CfgMap& cfg) {
  return val::to_string(v.name) + "=" + val::to_string(v.val);
}
string val::to_string(double d, const cfg::CfgMap& cfg) {
  auto digits = static_cast<size_t>(get<int64_t>(cfg.get("digits"s)));
  return ztsdb::to_string(d, digits);
}
string val::to_string(const string& s, const cfg::CfgMap& cfg, bool unquoted) {
  return unquoted ? s : '"' + s + '"';
}
string val::to_string(const val::integer_t& i, const cfg::CfgMap& cfg) {
  return std::to_string(i);
}
string val::to_string(const val::SpVI& s, const cfg::CfgMap& cfg) {
  stringstream ss;
  ss << "[";
  for (size_t j=0; j<s->size()-1; ++j) {
    ss << (*s)[j] << " ";
  }
  ss << (*s)[s->size()-1];
  ss << "]";
  return ss.str();
}
string val::to_string(bool b, const cfg::CfgMap& cfg) {
  return b ? "TRUE" : "FALSE";
}
string val::to_string(Global::dtime dt, const cfg::CfgMap& cfg, bool unquoted) {
  auto timezone = get<std::string>(cfg.get("timezone"s));
  // no format for the time being, so pass "", but we're covered if we
  // want to add it later:
  return tz::to_string(dt, "", tzones.find(timezone), timezone, true);
}
string val::to_string(Global::dtime::duration d, const cfg::CfgMap& cfg) {
  return tz::to_string(d);
}
string val::to_string(tz::interval i, const cfg::CfgMap& cfg) {
  auto timezone = get<std::string>(cfg.get("timezone"s));
  // no format for the time being, so pass "", but we're covered if we
  // want to add it later:
  return tz::to_string(i, "", tzones.find(timezone), timezone, true); // true == use the abbreviation
}
string val::to_string(tz::period p, const cfg::CfgMap& cfg) {
  return tz::to_string(p);
}
string val::to_string(const arr::zstring& s, const cfg::CfgMap& cfg, bool unquoted) {
  return unquoted ? s : "\"" + s + "\"";
}
string val::to_string(const VError& e, const cfg::CfgMap& cfg) {
  return e.what;
}
string val::to_string(const VPtr& p, const cfg::CfgMap& cfg) {
  return p.p == nullptr ? "vptr(null)" : to_string(*p.p);
}

template <>
string val::str(const val::VList& vl, const cfg::CfgMap& cfg, std::string prefix) 
{ 
  stringstream ss;
  ss << ' ' << val::Typeof()(vl) << " of " << vl.size();
  auto names = vl.getNamesVector(0);
  // make a function getMaxLength over the slice LLL
  // look at that kind of stuff in display.hpp too
  auto namesLength = getLengths(names);
  auto maxLen = names.size() ? *max_element(namesLength.begin(),namesLength.end()) : 0;
  for (idx_type i=0; i<std::min(vl.size(), 99UL); ++i) {
    ss << "\n" << prefix << " $ ";
    if (names.size()) {
      ss.width(maxLen);
      ss << left << names[i];
    }
    ss << ':' << str(vl[i], cfg, prefix + " .."s);
  }
  return ss.str();
}


template <>
inline string val::str(const arr::zts& z, const cfg::CfgMap& cfg, std::string prefix) {
  stringstream ss;
  ss << ' ' << val::Typeof()(z) << " : ";
  if (z.getIndex().size()) {
    ss << val::to_string(z.getIndex()[0], cfg) << " -> " 
       << val::to_string(z.getIndex()[z.getIndex().size()-1], cfg);
  }
  ss << "\n" << prefix << " - data =" << val::str(z.getArray(), cfg, prefix);
  return ss.str();

}


template <>
string val::str(const val::VClos& vl, const cfg::CfgMap& cfg, std::string prefix) 
{ 
  stringstream ss;
  ss << ' ' << val::Typeof()(vl) << '(' 
     << (vl.f->formlist ? to_string_cs(*vl.f->formlist) : "") 
     << ")";
  return ss.str();
}


string val::str(const val::Value& v, const cfg::CfgMap& cfg, std::string prefix) {

  // use visitor LLL
  
  stringstream ss;

  switch (v.which()) {
  case val::vt_list:
    ss << val::str(*get<const val::SpVList>(v), cfg, prefix);
    break;
  case val::vt_string:
    ss << val::str(*get<const val::SpVAS>(v), cfg, prefix);
    break;
  case val::vt_double:  
    ss << val::str(*get<const val::SpVAD>(v), cfg, prefix);
    break;
  case val::vt_bool:  
    ss << val::str(*get<const val::SpVAB>(v), cfg, prefix);
    break;
  case val::vt_time:  
    ss << val::str(*get<const val::SpVADT>(v), cfg, prefix);
    break;
  case val::vt_duration:  
    ss << val::str(*get<const val::SpVADUR>(v), cfg, prefix);
    break;
  case val::vt_interval:  
    ss << val::str(*get<const val::SpVAIVL>(v), cfg, prefix);
    break;
  case val::vt_period:  
    ss << val::str(*get<const val::SpVAPRD>(v), cfg, prefix);
    break;
  case val::vt_zts:  
    ss << val::str(*get<const val::SpZts>(v), cfg, prefix);
    break;
  case val::vt_clos:  
    ss << val::str(*get<const std::shared_ptr<val::VClos>>(v), cfg, prefix);
    break;
  default:
    ss << ' ' << val::to_string(v, cfg);
  }
  if (val::isLocked(v)) {
    ss << ", locked";
  }
  return ss.str();
}

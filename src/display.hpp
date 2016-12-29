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


#ifndef DISPLAY_HPP
#define DISPLAY_HPP


#include <limits>
#include "array.hpp"
#include "valuevar.hpp"
#include "zts.hpp"
#include "config.hpp"
#include "index.hpp"


namespace val {

  template<typename T>
  string to_string(T t, const cfg::CfgMap& cfg, bool fast=false);
  string to_string(const VCode& v, const cfg::CfgMap& cfg, bool fast=false);
  string to_string(const std::shared_ptr<VClos>& v, const cfg::CfgMap& cfg, bool fast=false);
  string to_string(const VClos& v, const cfg::CfgMap& cfg=cfg::cfgmap, bool fast=false);
  string to_string(const VNull& v, const cfg::CfgMap& cfg, bool fast=false);
  string to_string(const VBuiltinG& v, const cfg::CfgMap& cfg, bool fast=false);
  string to_string(const SpFuture& v, const cfg::CfgMap& cfg, bool fast=false);
  string to_string(const VNamed& v, const cfg::CfgMap& cfg, bool fast=false);
  string to_string(const VConn& conn, const cfg::CfgMap& cfg, bool fast=false);
  string to_string(const SpTimer& v, const cfg::CfgMap& cfg, bool fast=false);
  string to_string(const VList& l, const cfg::CfgMap& cfg, bool fast=false);
  string to_string(const SpVList& l, const cfg::CfgMap& cfg, bool fast=false);
  string to_string(double d, const cfg::CfgMap& cfg, bool fast=false);
  string to_string(bool, const cfg::CfgMap& cfg, bool fast=false);
  string to_string(const arr::zts& ts, const cfg::CfgMap& cfg, bool fast);
  string to_string(const SpZts& ts, const cfg::CfgMap& cfg, bool fast=false);
  string to_string(Global::dtime dt, const cfg::CfgMap& cfg, bool fast=false);
  string to_string(Global::dtime::duration d, const cfg::CfgMap& cfg, bool fast=false);
  string to_string(tz::interval i, const cfg::CfgMap& cfg, bool fast=false);
  string to_string(tz::period p, const cfg::CfgMap& cfg, bool fast=false);
  string to_string(const string& s, const cfg::CfgMap& cfg, bool fast=false);
  string to_string(const val::integer_t& s, const cfg::CfgMap& cfg, bool fast=false);
  string to_string(const val::SpVI& s, const cfg::CfgMap& cfg, bool fast=false);
  string to_string(const arr::zstring& s, const cfg::CfgMap& cfg, bool fast=false);
  string to_string(const VError& e, const cfg::CfgMap& cfg, bool fast=false);

  using namespace std::string_literals;

  // arrays -------------------------------------------------------


  // --- 
  template<typename T>
  inline Vector<zstring> vectorToString(const Vector<T>& v, const cfg::CfgMap& cfg) {
    Vector<zstring> res;
    transform(v.begin(), v.end(), back_inserter(res), 
              [cfg](const T& t) { return to_string(t, cfg); });
    return res;
  }

  template<>
  Vector<zstring> vectorToString(const Vector<double>& v, const cfg::CfgMap& cfg);

  template<>
  Vector<zstring> vectorToString(const Vector<Global::dtime>& v, const cfg::CfgMap& cfg);

  template<>
  Vector<zstring> vectorToString(const Vector<tz::interval>& v, const cfg::CfgMap& cfg);

  template<>
  Vector<zstring> vectorToString(const Vector<tz::period>& v, const cfg::CfgMap& cfg);

  template<typename T>
  struct ToString {
    ToString(const cfg::CfgMap& cfg_p) : cfg(cfg_p) { }
    Vector<zstring> operator()(Vector<T> v) { return vectorToString(v, cfg); }
    const cfg::CfgMap& cfg;
  };

  template<typename T>
  inline Array<zstring> arrayToString(const Array<T>& a, const cfg::CfgMap& cfg) {
    return Array<zstring>(a, ToString<T>(cfg), true);
  }

  template<>
  Array<zstring> arrayToString(const Array<double>& a, const cfg::CfgMap& cfg);

  template<>
  Array<zstring> arrayToString(const Array<Global::dtime>& a, const cfg::CfgMap& cfg);

  template<>
  Array<zstring> arrayToString(const Array<tz::interval>& a, const cfg::CfgMap& cfg);

  template<>
  Array<zstring> arrayToString(const Array<tz::period>& a, const cfg::CfgMap& cfg);

  template<typename T>
  Vector<zstring> getDimnames(const Array<T>&a, 
                              idx_type d, 
                              idx_type n,
                              const string& prefix="",
                              const string& postfix="") {
    if (a.names.size() > d && a.hasNames(d)) {
      return Vector<zstring>(a.names[d]->names.begin(), a.names[d]->names.begin()+n);
    } else {
      Vector<zstring> vs;
      for (idx_type j=0; j<n; ++j) {
        vs.push_back("[" + prefix + std::to_string(j+1) + postfix + "]");
      }
      return vs;
    }
  }

  inline Vector<zstring> getVectorLineNumbers(idx_type n, idx_type ncols) {
    Vector<zstring> vs;
    auto nrows = n / ncols;
    if (n % ncols) {
      ++nrows;
    }
    for (idx_type j=0; j<nrows; ++j) {
      vs.push_back("[" + std::to_string(1 + j*ncols) + "]");
    }
    return vs;
  }

  inline vector<unsigned> getLengths(const Vector<zstring> vs) {
    auto vl = vector<unsigned>();
    transform(vs.begin(), vs.end(), back_inserter(vl), [](string s){ return s.length(); });
    return vl;
  }

  template<typename T>
  string displayVector(const Array<T>&a_p, const cfg::CfgMap& cfg) {
    unsigned cfgWidth = static_cast<size_t>(get<int64_t>(cfg.get("width")));
    auto cfgMaxPrint = static_cast<arr::idx_type>(get<int64_t>(cfg.get("max.print")));

    auto a = a_p.size() > cfgMaxPrint ? a_p.subsetRows(cfgMaxPrint) : a_p;

    stringstream ss;
    idx_type n = min(a.dim[0], cfgMaxPrint);
    auto vs = vectorToString(*a.v[0], cfg);

    auto lenValues = getLengths(vs);

    auto colstring = a.hasNames(0) ? getDimnames(a, 0, n) : Vector<zstring>();
    unsigned colWidth;
    if (a.hasNames(0)) {
      auto lenNames  = getLengths(colstring);
      colWidth = std::max(*max_element(lenNames.begin(),  lenNames.end()),
                          *max_element(lenValues.begin(), lenValues.end())) + 1;
    } else {
      colWidth = *max_element(lenValues.begin(), lenValues.end()) + 1;
    }

    // figure out the line breaks iteratively:
    auto ncols = min(cfgWidth / colWidth, static_cast<unsigned>(n));
    if (!ncols) ++ncols;        // we have to display something!
    auto vln = getVectorLineNumbers(n, ncols);
    auto lenVln = getLengths(vln);
    auto vlnWidth = *max_element(lenVln.begin(), lenVln.end());
    while (ncols > 1 && vlnWidth + ncols*colWidth > cfgWidth) {
      --ncols;         // it now doesn't fit anymore, so decrease ncol
      vln = getVectorLineNumbers(n, ncols);
      lenVln = getLengths(vln);
      vlnWidth = *max_element(lenVln.begin(), lenVln.end());
    }
    auto nrows = n / ncols;
    if (n % ncols) ++nrows;
    for (unsigned j=0; j<nrows; ++j) {
      if (a.hasNames(0)) {
        // print the names:
        ss.width(vlnWidth);
        ss << "";
        for (unsigned k=0; k<ncols; ++k) {
          ss << ' ';
          if (k+1 + j*ncols < n) {
            ss.width(colWidth-1);
            ss << left << colstring[k + j*ncols];
          }
          else {
            ss << left << colstring[k + j*ncols];
            break;
          }
        }
        ss << endl;
      }
      // print the data:
      ss.width(vlnWidth);
      ss << right << vln[j];
      for (unsigned k=0; k<ncols; ++k) {
        ss << ' ';
        if (k+1 + j*ncols < n) {
          ss.width(colWidth-1);
          ss << left << vs[k + j*ncols];
        }
        else {
          ss << left << vs[k + j*ncols];
          break;
        }
      }
      if (j<nrows-1) ss << endl; // no trailing line break
    }

    // if we didn't print everything, then let the user know
    if (n < a_p.dim[0]) {
      ss << endl << " [ reached getOption(""max.print"") -- omitted " << 
        a_p.dim[0] - n << " entries ]";
    }

    return ss.str();
  }

  // MAXIDX is used to indicate we have a zero index, which needs to
  // be differentiated so as to display '0' and not '1'!
  const idx_type MAXIDX = std::numeric_limits<idx_type>::max();
  inline vector<Index> getFirstVIndex(const arr::Vector<idx_type>& dim, idx_type maxrows) {
    vector<Index> fi;
    if (dim.size() < 2) {
      throw std::out_of_range("getFirstVIndex(): dim.size() < 2");    
    }
    if (maxrows < dim[0]) {
      Vector<size_t> v(std::max(static_cast<idx_type>(1), maxrows));
      std::iota(v.begin(), v.end(), 0);
      fi.push_back(IntIndex(v));
      fi.push_back(NullIndex{dim[1]});
    } else {
      fi.push_back(NullIndex{dim[0]});
      fi.push_back(NullIndex{dim[1]});
    }
    for (idx_type j=2; j<dim.size(); ++j) {
      fi.push_back(IntIndex(Vector<size_t>{ dim[j] ? 0 : size_t(MAXIDX) }));
    }
    return fi;
  }

  inline bool getNextVIndex(vector<Index>& i, const arr::Vector<idx_type>& dim) {
    if (dim.size() < 2) {
      throw std::out_of_range("getNextVIndex(): dim.size() < 2");
    }
    if (i.size() != dim.size()) {
      throw std::out_of_range("getNextVIndex(): index and dim sizes mismatch");
    }
    for (idx_type j=dim.size()-1; j>=2; --j) {
      if (get<IntIndex>(i[j].idx).vi[0] == MAXIDX) {
        continue;
      } else if (get<IntIndex>(i[j].idx).vi[0] + 1 >= dim[j]) {
        auto& ref = get<IntIndex>(i[j].idx);
        arr::setv(ref.vi, 0, 0UL);
      } else {
        auto& ref = get<IntIndex>(i[j].idx);
        arr::setv(ref.vi, 0, (ref.vi)[0]+1);
        return true;
      }
    }
    return false;
  }

  inline string displaySliceHeader(const vector<Index>& ii, const vector<unique_ptr<Dname>>& names) {
    stringstream ss;
    if (ii.size() < 2) {
      throw std::out_of_range("displaySliceHeader(): ii.size() < 3");
    }
    ss << ", ";
    for (idx_type j=2; j<ii.size(); ++j) {
      auto idx = get<IntIndex>(ii[j].idx).vi[0];
      if (j < names.size()) {
        ss << ", " << (idx == MAXIDX  ? "0" :
                       (names[j]->size() ? (*names[j])[idx] : std::to_string(idx+1)));
      }
    }
    ss << "\n";
    return ss.str();
  }

  template<typename T, typename R>
  string displaySlice(const Array<T>& slice, 
                      const Vector<R>& rownames_c, 
                      const cfg::CfgMap& cfg, 
                      idx_type& nleft) 
  {
    auto cfgWidth = static_cast<size_t>(get<int64_t>(cfg.get("width")));

    stringstream ss;
    
    auto nrows = min(nleft / std::max(slice.dim[1], static_cast<idx_type>(1)), slice.dim[0]);
    auto rownames = Vector<zstring>(nrows);
    if (rownames_c.size() == 0) {
      for (idx_type j=0; j<nrows; ++j) {
        arr::setv(rownames, j, arr::zstring("[" + std::to_string(j+1) + ",]"));
      }
    } else {
      for (idx_type j=0; j<nrows; ++j) {
        arr::setv(rownames, j, arr::zstring(to_string(rownames_c[j], cfg, true)));
      }
    }

    auto lenRownames = getLengths(rownames);
    auto colnames = getDimnames(slice, 1, min(nleft, slice.dim[1]), ",");
    auto lenColnames = getLengths(colnames);
    auto data = arrayToString(slice, cfg);
    auto lenData = applyf<arr::zstring, unsigned>(data, [](const arr::zstring& s){ return s.length(); });
    auto colWidths = maxcol(lenData);
    for (idx_type j=0; j<lenColnames.size(); ++j) {
      arr::setv(colWidths, j, std::max(colWidths[j], lenColnames[j]));
    }
    colWidths = applyf<unsigned, unsigned>(colWidths, [](unsigned u) { return ++u; });
    auto rownamesWidth = lenRownames.size() ? 
      *max_element(lenRownames.begin(), lenRownames.end()) : 0;
    
    idx_type colStart = 0;
    while (colStart < colnames.size()) {
      // figure out where we need to line break:
      idx_type colEnd = colStart + 1;      // print at least one!
      unsigned nchar = rownamesWidth + colWidths[colEnd-1];
      while (colEnd < colnames.size()) { // use colnames in case we are truncating the columns
        nchar += colWidths[colEnd];
        if (nchar > cfgWidth) {
          break;
        } else {
          ++colEnd;
        }
      }
      // print the column header:
      ss.width(rownamesWidth);
      ss << "";
      for (idx_type col=colStart; col<colEnd; ++col) {
        ss.width(colWidths[col]);
        ss << colnames[col];
      }
      // if at the end but not all the columns printed, print an ellipsis:
      if (colEnd == colnames.size() && colEnd < slice.dim[1]) {
        ss << " ...";
      }
      ss << endl;
      // print the data:
      for (idx_type row=0; row<rownames.size(); ++row) {
        ss.width(rownamesWidth);
        ss << (rownames_c.size() ? left : right) << rownames[row] << right;
        for (idx_type col=colStart; col<colEnd; ++col) {
          ss << " ";
          ss.width(colWidths[col]-1);
          ss << left << data[row + col * slice.dim[0]] << right;
        }
        ss << endl;
      }
      colStart = colEnd;
    }

    nleft -= nrows * std::max(slice.dim[1], static_cast<idx_type>(1));
    return ss.str().substr(0, ss.str().length()-1); // pull out trailing line break
  }


  // we could probably slice with zeros and consolidate this function
  // and the displaySlice... LLL
  template<typename R>          // R: row names type
  string displaySliceWithZeros(const vector<Index>& i,
                               const arr::Vector<idx_type>& dim,
                               const vector<unique_ptr<Dname>>& names, 
                               const Vector<R>& rownames_c, 
                               const cfg::CfgMap& cfg, 
                               idx_type& nleft) 
  {
    stringstream ss;

    auto nrows = min(nleft / std::max(dim[1], static_cast<idx_type>(1)), dim[0]);
    auto rownames = Vector<zstring>(nrows);
    if (rownames_c.size() == 0) {
      for (idx_type j=0; j<nrows; ++j) {
        arr::setv(rownames, j, arr::zstring("[" + std::to_string(j+1) + ",]"));
      }
    } else {
      for (idx_type j=0; j<nrows; ++j) {
        arr::setv(rownames, j, arr::zstring(to_string(rownames_c[j], cfg, true)));
      }
    }
    auto lenRownames = getLengths(rownames);
    auto rownamesWidth = lenRownames.size() ? 
      *max_element(lenRownames.begin(), lenRownames.end()) : 0;
 
    auto ncols = min(nleft, std::max(dim[1], static_cast<idx_type>(1)));
    auto colnames = names[1]->names;
    if (colnames.size() == 0) {
      for (idx_type j=0; j<ncols; ++j) {
        colnames.push_back("[," + std::to_string(j+1) + "]");
      }
    } else {
      colnames.resize(ncols);
    }
    auto lenColnames = getLengths(colnames);
    auto colnamesWidth = lenColnames;
    for_each(colnamesWidth.begin(), colnamesWidth.end(), [](unsigned& x) { ++x; });

    idx_type colStart = 0;
    auto cfgWidth = static_cast<size_t>(get<int64_t>(cfg.get("width")));
    while (colStart < colnames.size()) {
      // figure out where we need to line break:
      idx_type colEnd = colStart + 1;      // print at least one!
      unsigned nchar = rownamesWidth + lenColnames[colEnd-1];
      while (colEnd < colnames.size()) { // use colnames in case we are truncating the columns
        nchar += colnamesWidth[colEnd];
        if (nchar > cfgWidth) {
          break;
        } else {
          ++colEnd;
        }
      }
      if (dim[1]) {
        // print the column header:
        ss.width(rownamesWidth);
        ss << "";
        for (idx_type col=colStart; col<colEnd; ++col) {
          ss.width(colnamesWidth[col]);
          ss << colnames[col];
        }
        // if at the end but not all the columns printed, print an ellipsis:
        if (colEnd == colnames.size() && colEnd < dim[1]) {
          ss << " ...";
        }
        ss << endl;
      }
      // print the rownames:
      for (idx_type row=0; row<rownames.size(); ++row) {
        ss.width(rownamesWidth);
        ss << (rownames_c.size() ? left : right) << rownames[row] << right;
        ss << endl;
      }
      colStart = colEnd;      
    }

    nleft -= nrows * ncols;

    return ss.str().substr(0, ss.str().length()-1); // pull out trailing line break
  }


  template<typename T, typename R>
  string display(const Array<T>& a, const Vector<R>& rownames, const cfg::CfgMap& cfg) {
    auto cfgMaxPrint = static_cast<size_t>(get<int64_t>(cfg.get("max.print")));

    stringstream ss;

    if (!a.dim.size()) {
      throw domain_error("array with no dimensions");
    }

    // if all dimensions are 0 just display that:
    if (all_of(a.dim.begin(), a.dim.end(), [](idx_type x) { return x==0; })) {
      ss << arr::TypeName<T>::s << '(';
      copy(a.dim.begin(), a.dim.end() - 1, ostream_iterator<idx_type>(ss, "x"));
      ss << a.dim.back();
      ss << ")";
      return ss.str();
    }

    // the one dimentional case really has a set of rules of its own,
    // so treat it separately:
    if (a.dim.size() == 1) {
      return displayVector(a, cfg);
    }

    // if we have a 0 dimension somewhere but with other non 0
    // dimensions, we only print header and row/col name information;
    // it's a special case, so treat is separately: 
    bool hasZeros = std::any_of(a.dim.begin(), a.dim.end(), [](idx_type x) { return x==0; });

    // replace dimensions of size 0 with 1 so we can calculate
    // correctly what we need to display (for example, if we have
    // columns==0, we will still display the rows, so we will display
    // rows x 1):
    auto dim1 = a.dim;
    for_each(dim1.begin(), dim1.end(), [](idx_type& x) { if (!x) x = 1; });

    // how many slices?:
    idx_type totslices, nslices, ommittedSlices;
    idx_type sliceSize = dim1[0] * dim1[1];
    idx_type tot = accumulate(dim1.begin(), dim1.end(), 1, std::multiplies<idx_type>());
    if (a.dim.size() == 2) {
      totslices = nslices = 1;
      ommittedSlices = 0;
    } else {
      totslices = accumulate(dim1.begin()+2, dim1.end(), 1, std::multiplies<idx_type>());
      nslices = min(totslices, cfgMaxPrint / sliceSize);
      if (min(tot, static_cast<idx_type>(cfgMaxPrint)) % sliceSize) {
        ++nslices;              // the slice is not ommitted, it's partially printed
      }
      ommittedSlices = totslices - nslices;
    }

    auto nleft = min(tot, static_cast<idx_type>(cfgMaxPrint));
    idx_type ommittedRows = tot <= cfgMaxPrint ? 0 : (dim1[0] - nleft % sliceSize / dim1[1]);
    if (cfgMaxPrint >= dim1[1]) {
      ommittedRows %= dim1[0];
    }

    idx_type maxrows = min(dim1[0], nleft / dim1[1]);
    auto vi = getFirstVIndex(a.dim, maxrows);
    for (idx_type k=0; k<nslices; ++k) {
      if (totslices > 1) {
        ss << displaySliceHeader(vi, a.names) << endl;
      }
      if (hasZeros) {
        ss << displaySliceWithZeros(vi, a.dim, a.names, rownames, cfg, nleft) << endl;
      } else {
        // drop=false, because we don't want to drop to a vector;
        // displaySlice handles correctly trailing dims of size 1:
        auto slice = a(vi, false); 
        ss << displaySlice(slice, rownames, cfg, nleft) << endl;
      }
      if (totslices > 1) {
        ss << endl;
      }
      getNextVIndex(vi, dim1);
    }

    // if we didn't print everything, then let the user know
    if (ommittedRows && ommittedSlices) {
      ss << " [ reached getOption(""max.print"") -- omitted "
         << ommittedRows << " row(s) and " << ommittedSlices << " slice(s) ]" << endl;
    } else if (ommittedRows) {
      ss << " [ reached getOption(""max.print"") -- omitted "
         << ommittedRows << " row(s) ]" << endl;
    } else if (ommittedSlices) {
      ss << " [ reached getOption(""max.print"") -- omitted "
         << ommittedSlices << " slice(s) ]" << endl;
    }

    return ss.str().substr(0, ss.str().length()-1); // pull out trailing line break
  }


  string display(const SpVList& l, const cfg::CfgMap& cfg, string prefix);

  template<typename T>
  const string to_string(const Array<T>& v, const cfg::CfgMap& cfg, bool fast=false) {
    return display(v, v.names[0]->names, cfg); 
  }

  template<typename T>
  const string to_string(const cow_ptr<Array<T>>& v, const cfg::CfgMap& cfg, bool fast=false) {
    return display(*v, v->names[0]->names, cfg); 
  }


  // ---------------------------
  // to_string - visitor to print out values:
  struct to_string_v {
    typedef string result_type;

    template <typename T>
    string operator()(const T& t, const cfg::CfgMap& cfg, bool fast=false) const { 
      return to_string(t, cfg, fast); 
    }
  };

  inline string to_string(const val::Value& v, const cfg::CfgMap& cfg=cfg::cfgmap, bool fast=false) { 
    return apply_visitor(to_string_v(), v, cfg, fast); 
  }

  string display(const val::Value& v);

  // str --------------------------
  string str(const val::Value& v, const cfg::CfgMap& cfg, std::string prefix="");
  
  // visitor to print out short descritption of values:
  template <typename T>
  string str(const T& v, const cfg::CfgMap& cfg, std::string prefix) { 
    auto cfgWidth = static_cast<size_t>(get<int64_t>(cfg.get("width")));

    stringstream ss;
    ss << ' ' << val::Typeof()(v);
    if (v.isOrdered()) {
      ss << " - ord";
    }
    if (!v.isScalar()) {
      // dimensions:
      ss << " [";
      for (arr::idx_type i=0; i<v.getdim().size(); ++i) {
        stringstream dims;
        dims << "1:" << v.getdim(i);
        if (i != v.getdim().size()-1) {
          dims << ", ";
        }
        if (ss.str().size() + dims.str().size() > cfgWidth-4) {
          ss << "...";
          break;
        }
        else {
          ss << dims.str();
        }
      }
      ss << "]";
    }
    ss << ' ';
    // elts
    for (arr::idx_type i=0; i<v.size(); ++i) {
      stringstream elt;
      elt << to_string(v[i], cfg);
      if (ss.str().size() + elt.str().size() > cfgWidth-4) {
        ss << "...";
        break;
      }
      else {
        ss << elt.str();
        if (i != v.size()-1) ss << ' ';
      }
    }
    // dimnames:
    // build a vlist:
    bool hasDimNames = false;
    auto dimnames = make_cow<val::VList>(false);
    for (arr::idx_type i=0; i<v.getdim().size(); ++i) {
      if (v.hasNames(i)) {
        auto dimarray = make_cow<val::VArrayS>(false, 
                                               arr::Vector<arr::idx_type>{v.getdim(i)},
                                               v.getNames(i).names);
        dimnames->push_back(std::make_pair(arr::zstring(""), val::Value(dimarray)));
        hasDimNames = true;
      }
      else {
        dimnames->push_back(std::make_pair(arr::zstring(""), val::VNull()));
      }
    }
    if (hasDimNames) {
      ss << std::endl << prefix << " - dimnames =" << str(val::Value(dimnames), cfg, prefix + " ..");
    }
    ss << std::endl << prefix << " - " << v.getAllocFactory().to_string();
    return ss.str();
  }

  template <>
  string str(const val::VList& vl, const cfg::CfgMap& cfg, std::string prefix);

  template <>
  string str(const arr::zts& z, const cfg::CfgMap& cfg, std::string prefix);

  template <>
  string str(const val::VClos& vl, const cfg::CfgMap& cfg, std::string prefix);

}


#endif

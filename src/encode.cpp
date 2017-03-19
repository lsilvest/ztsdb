// (C) 2016,2017 Leonardo Silvestri
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


#include "encode.hpp"
#include "misc.hpp"
#include "globals.hpp"
#include "display.hpp"


// #define DEBUG

/// \cond
static const unsigned STRALIGN = 8;             // make string lengths multiples of STRALIGN
static const unsigned VNAMED_LEN = 2;		// always 2 elements in a VNamed

// the following are constants determining the encoding sizes.
using ETYPE_T    = uint64_t;
using INT_T      = uint64_t;
using DTIME_T    =  int64_t;
using BOOL_T     = uint64_t;
using DOUBLE_T   = uint64_t;
using OP_T       = uint64_t; // in Unop and Binop
using EL_N_T     = uint64_t; // El::n
using STRLEN_T   = uint64_t;
using CODELEN_T  = uint64_t;
using VLISTN_T   = uint64_t;
using IVL_OPEN_T = uint32_t;


// some assertions that must hold so we don't truncate during encoding:
static_assert(sizeof(BOOL_T)   >= sizeof(Bool::value_type),   "BOOL_T encoding too small");
static_assert(sizeof(DOUBLE_T) >= sizeof(Double::value_type), "DOUBLE_T encoding too small");
static_assert(sizeof(OP_T)     >= sizeof(Unop::op),     "OP_T(Unop) encoding too small");
static_assert(sizeof(OP_T)     >= sizeof(Binop::op),    "OP_T(Binop) encoding too small");
static_assert(sizeof(EL_N_T)   >= sizeof(El::n),        "EL_N_T(El::n) encoding too small");
/// \endcond


/// Convert a 'vector' of 'Dname' to a list of strings. This is used
/// to encode the 'names' field of an 'Array'. The result of the
/// function is a list of the same length as the number of dimensions,
/// where each element of this list is itself a list. These sublists
/// are the names of the corresponding dimension.
val::Value zcore::convertToList(const vector<unique_ptr<arr::Dname>>& names) {
#ifdef DEBUG
  cout << "val::Value zcore::convertToList(const vector<arr::Dname>& names)" << endl;
#endif
  auto nameslist = make_cow<val::VList>(arr::NOFLAGS);
  for (idx_type j=0; j<names.size(); ++j) {
    auto dimnameslist = make_cow<val::VList>(arr::NOFLAGS);
    for (arr::idx_type k=0; k<names[j]->names.size(); ++k) {
      dimnameslist->push_back(make_pair("", std::string((*names[j])[k])));
    }
    nameslist->push_back(make_pair("", dimnameslist));
  }
  return nameslist;
}


zcore::Encode& zcore::Encode::operator <<(const val::VI& t) {
  *this << t.size();
  for (size_t j=0; j < t.size(); ++j) {
    *this << t[j];
  }
  return *this;
}

zcore::Encode& zcore::Encode::operator <<(const string& t) {
#ifdef DEBUG
  cout << "zcore::Encode::operator <<(const string& t)" << endl;
  cout << "| string: " << t << endl;
#endif
  *this << static_cast<const STRLEN_T&>(t.length());

  size_t stringOffset = 0;
  while (stringOffset < t.length()) {
    const unsigned nmax = Global::CHUNKSZ - offset;
    size_t remaining = t.length() - stringOffset;
    if (remaining < nmax) {
      memcpy(buf + offset, t.c_str(), remaining);
      offset += remaining;
      auto alignBytes = getAlignedLength(remaining, STRALIGN) - remaining;
      memset(buf + offset, 0, alignBytes);
      offset += alignBytes;
      return *this;
    } else {
      memcpy(buf + offset, t.c_str(), nmax);
      auto res = com.send(peerid, buf, offset);
      if (res < 0) {
        throw std::system_error(std::error_code(errno, std::system_category()), "Encode(string)");
      }
      stringOffset += nmax;
      bytes += res;           // just stats
    }
  }
  return *this;
}


zcore::Encode& zcore::Encode::operator <<(const arr::zstring& t) {
  return operator<<(static_cast<std::string>(t));
}


zcore::Encode& zcore::Encode::operator <<(const tz::interval& t) {
  return *this << t.s << t.e << t.sopen << t.eopen;
}


zcore::Encode& zcore::Encode::operator <<(const tz::period& t) {
  return *this << t.getMonths() << t.getDays() << t.getDuration();
}


zcore::Encode& zcore::Encode::operator <<(const arr::zts& t) {
  *this << uint64_t(2);              // 2 elts: data and idx
  *this << uint64_t(val::vt_double); // force nesting - stage 0
  *this << t.getArray();
  *this << uint64_t(val::vt_time);   // force nesting - stage 1
  *this << t.getIndex();
  return *this;
}


zcore::Encode& zcore::Encode::operator <<(const bool t) {
  return *this << static_cast<BOOL_T>(t);
}


zcore::Encode& zcore::Encode::operator <<(const val::Value& v) {
#ifdef DEBUG
  cout << "zcore::Encode::operator<<(const val::Value& v)" << endl;
  cout << "| Value type is " << val::vt_to_string.at(v.which()) << endl;
  cout << "| Value is " << val::display(v) << endl;
#endif

  if (offset + 16 >= Global::CHUNKSZ) {
    flush();
  }  
  *this << uint64_t(v.which());
  switch (v.which()) {
  case val::vt_list: {
    const auto& vl = get<val::SpVList>(v);
    *this << static_cast<const VLISTN_T&>(vl->size());
    for (size_t i=0; i<vl->size(); ++i) {
      *this << val::Value(val::VNamed{vl->a.getnames(0)[i], // name
                                     (vl->a)[i]}); // value
    }
    break;
  }

  case val::vt_std_string: {
    const auto& va = get<std::string>(v);
    *this << va;
    break;
  }
  case val::vt_std_int: {
    const auto& va = *get<val::SpVI>(v);
    *this << va;
    break;
  }
  case val::vt_string: {
    const auto& va = *get<val::SpVAS>(v);
    *this << va;
    break;
  }
  case val::vt_double: {
    const auto& va = *get<val::SpVAD>(v);
    *this << va;
    break;
  }
  case val::vt_bool: {
    const auto& va = *get<val::SpVAB>(v);
    *this << va;
    break;
  } 
  case val::vt_time: {
    const auto& va = *get<val::SpVADT>(v);
    *this << va;
    break;
  }
  case val::vt_duration: {
    const auto& va = *get<val::SpVADUR>(v);
    *this << va;
    break;
  }
  case val::vt_interval: {
    const auto& va = *get<val::SpVAIVL>(v);
    *this << va;
    break;
  }
  case val::vt_period: {
    const auto& va = *get<val::SpVAPRD>(v);
    *this << va;
    break;
  }
  case val::vt_zts: {
    const auto& vz = *get<val::SpZts>(v);
    *this << vz;
    break;
  }
  case val::vt_null:
    *this << static_cast<uint64_t>(0); // encode length 0
    break;

  case val::vt_clos:
  case val::vt_builting:
  case val::vt_future:
  case val::vt_connection: {
    throw std::domain_error(string("Encode::operator <<(const Value& v): not implemented for ") + 
                            std::to_string(v.which()));
  }
  case val::vt_named: {
    *this << static_cast<const VLISTN_T&>(VNAMED_LEN);
    const auto& n = get<val::VNamed>(v);
    *this << n.name;
    *this << n.val;
    break;
  }
  case val::vt_error: {
    const auto& e = get<val::VError>(v);
    *this << e.what;
    break;    
  }
  default:
    throw std::domain_error(string("Encode::operator <<(const Value& v): unknown type ") + 
                            std::to_string(v.which()));
  }
  return *this;
}


static zcore::Encode& writeCode(zcore::Encode& ec, const E* e) {
#ifdef DEBUG
  cout << "zcore::Encode::writeCode" << endl;
  cout << "| e->etype:  " << et_to_string[e->etype] << endl;
  cout << "| ec.offset: " << ec.offset << endl;
#endif
  
  ec << uint64_t(e->etype);
  switch (e->etype) {
  case etempty   : 
  case etnull    : 
  case etbreak   : 
  case etnext    : 
    break;
  case etbool    : {
    auto b = static_cast<const Bool*>(e);
    ec << static_cast<BOOL_T>((*b->data)[0]); // cast as it needs to be encoded over 8 bytes
    break;
  }
  case etdouble  : {
    auto b = static_cast<const Double*>(e);
    ec << (*b->data)[0];
    break;
  }
  case etdtime   : {
    auto d = static_cast<const Dtime*>(e);
    ec << (*d->data)[0];
    break;
  }
  case etinterval : {
    auto d = static_cast<const Interval*>(e);
    ec << (*d->data)[0];
    break;
  }
  case etsymbol  : {
    auto s = static_cast<const Symbol*>(e);
    ec << s->data;
    ec << static_cast<BOOL_T>(s->ref); // cast as it needs to be encoded over 8 bytes
    break;
  }
  case etstring  : {
    auto s = static_cast<const String*>(e);
    ec << (*s->data)[0];
    break;
  }
  case etboundvar: {
    auto bv = static_cast<const Boundvar*>(e);
    ec << bv->data;
    break;
  }
  case etunop : {
    auto u = static_cast<const Unop*>(e);
    ec << static_cast<const OP_T&>(u->op);
    writeCode(ec, u->e);
    break;
  }
  case etbinop : {
    auto b = static_cast<const Binop*>(e);
    ec << static_cast<const OP_T&>(b->op);
    writeCode(ec, b->left);
    writeCode(ec, b->right);
    break;
  }
  case etexprlist : {
    auto el = static_cast<const El*>(e);
    ec << uint64_t(el->n);
    auto eln = el->begin;
    for (unsigned n=0; n<el->n; ++n) {
      writeCode(ec, eln->e);
      eln = eln->next;
    }
    break;
  }
  case etwhile: {
    auto i = static_cast<const While*>(e);
    writeCode(ec, i->e1);
    writeCode(ec, i->e2);
    break;    
  }
  case etifelse: {
    auto ie = static_cast<const IfElse*>(e);
    writeCode(ec, ie->e1);
    writeCode(ec, ie->e2);
    writeCode(ec, ie->e3);
    break;    
  }
  case etfor: {
    auto f = static_cast<const For*>(e);
    writeCode(ec, f->forloop);
    break;
  }
  case etleftassign: {
    auto la = static_cast<const LeftAssign*>(e);
    writeCode(ec, la->e1);
    writeCode(ec, la->e2);
    break;    
  }
  case etspecialassign: {
    auto sa = static_cast<const SpecialAssign*>(e);
    writeCode(ec, sa->e1);
    writeCode(ec, sa->e2);
    break;    
  }
  case etrequest: {
    auto r = static_cast<const Request*>(e);
    writeCode(ec, r->e1);
    writeCode(ec, r->e2);
    break;    
  }
  case ettaggedexpr: {
    auto te = static_cast<const TaggedExpr*>(e);
    writeCode(ec, te->symb);
    writeCode(ec, te->e);
    break;
  }
  case etarg: {
    auto a = static_cast<const Arg*>(e);
    ec << static_cast<BOOL_T>(a->symb->ref); // cast as it needs to be encoded over 8 bytes
    writeCode(ec, a->e);
    break;
  }
  case etfunction: {
    auto f = static_cast<const Function*>(e);
    writeCode(ec, f->formlist);
    writeCode(ec, f->body);
    break;
  }
  case etfuncall: {
    auto es = static_cast<const Funcall*>(e);
    writeCode(ec, es->e);
    writeCode(ec, es->el);
    break;    
  }
  default:
    throw std::domain_error(string("writeCode: unknow type ") + 
                            std::to_string(e->etype));
  }  
  return ec;
}

zcore::Encode& zcore::Encode::operator <<(const E* e) {
#ifdef DEBUG
  cout << "zcore::Encode::operator <<():" << endl;
  cout << "| e ptr=" << e << endl;
  cout << "| with e=" << to_string(*e) << endl;
  cout << "| never get here?" << endl;
#endif
  
  *this << static_cast<CODELEN_T>(codeLength(e));
  return writeCode(*this, e);
}


static E* readCode(const char* buf, size_t len, size_t& off) {
  Etype etype = Etype(ntoh64(*(reinterpret_cast<const uint64_t*>(buf+off))));
  off += 8;
#ifdef DEBUG
  cout << "zcore::readCode() with len=" << len-off << endl;
  cout << "| etype: " << et_to_string[etype] << endl;
#endif

  switch (etype) {
  case etnull    : return new Null(yy::missing_loc());
  case etbreak   : return new Break(yy::missing_loc());
  case etnext    : return new Next(yy::missing_loc());
  case etbool    : {
    bool b = ntoh<BOOL_T>(*(reinterpret_cast<const BOOL_T*>(buf+off)));
    off += sizeof(BOOL_T);
    return new Bool(b, yy::missing_loc());
  }
  case etdouble  : {
    double d = ntoh<double>(*(reinterpret_cast<const double*>(buf+off)));
    off += sizeof(double);
    return new Double(d, yy::missing_loc());
  }
  case etdtime   : {
    Global::dtime d = ntoh<Global::dtime>(*(reinterpret_cast<const Global::dtime*>(buf+off)));
    off += sizeof(Global::dtime);
    return new Dtime(d, yy::missing_loc());
  }
  case etinterval  : {
    Global::dtime d1 = ntoh<Global::dtime>(*(reinterpret_cast<const Global::dtime*>(buf+off)));
    off += sizeof(Global::dtime);
    Global::dtime d2 = ntoh<Global::dtime>(*(reinterpret_cast<const Global::dtime*>(buf+off)));
    off += sizeof(Global::dtime);
    auto s1 = ntoh<uint32_t>(*(reinterpret_cast<const uint32_t*>(buf+off)));
    off += sizeof(uint32_t);
    auto s2 = ntoh<uint32_t>(*(reinterpret_cast<const uint32_t*>(buf+off)));
    off += sizeof(uint32_t);
    return new Interval(tz::interval(d1,d2,s1,s2), yy::missing_loc());
  }
  case etsymbol  : {
    unsigned strlen = ntoh<STRLEN_T>(*(reinterpret_cast<const STRLEN_T*>(buf+off)));
    off += sizeof(STRLEN_T);
    const string s(buf+off, buf+off+strlen);
    off += getAlignedLength(s.size(), STRALIGN);
    bool ref = ntoh<BOOL_T>(*(reinterpret_cast<const BOOL_T*>(buf+off)));
    off += sizeof(BOOL_T);
    return new Symbol(s, yy::missing_loc(), ref);
  }
  case etstring  : {
    unsigned strlen = ntoh<STRLEN_T>(*(reinterpret_cast<const STRLEN_T*>(buf+off)));
    off += sizeof(STRLEN_T);
    const arr::zstring s(buf+off, buf+off+strlen);
    off += getAlignedLength(s.size(), STRALIGN);
    return new String(s, yy::missing_loc());
  }
  case etboundvar: {
    unsigned strlen = ntoh<STRLEN_T>(*(reinterpret_cast<const STRLEN_T*>(buf+off)));
    off += sizeof(STRLEN_T);
    const string s(buf+off, buf+off+strlen);
    off += getAlignedLength(s.size(), STRALIGN);
    return new Boundvar(s, yy::missing_loc());
  }
  case etunop : {
    unsigned op = unsigned(ntoh64(*(reinterpret_cast<const uint64_t*>(buf+off))));
    off += 8;
    return new Unop(op, readCode(buf, len, off), yy::missing_loc());
  }
  case etbinop : {
    unsigned op = unsigned(ntoh64(*(reinterpret_cast<const uint64_t*>(buf+off))));
    off += 8;
    return new Binop(op, 
                     readCode(buf, len, off), 
                     readCode(buf, len, off), 
                     yy::missing_loc());
  }
  case etexprlist : {
    unsigned n = unsigned(ntoh64(*(reinterpret_cast<const uint64_t*>(buf+off))));
    off += 8;
    El* el = new El();
    for (unsigned i=0; i<n; ++i) {
      el->add(readCode(buf, len, off));
    }
    return el;
  }
  case etwhile:
    return new While(readCode(buf, len, off), 
                                   readCode(buf, len, off),
                                   yy::missing_loc());
  case etifelse:
    return new IfElse(readCode(buf, len, off), 
                                    readCode(buf, len, off),
                                    readCode(buf, len, off),
                                    yy::missing_loc());
  case etfor:
    return new For(readCode(buf, len, off), 
                                 yy::missing_loc());
  case etleftassign: 
    return new LeftAssign(readCode(buf, len, off), 
                                        readCode(buf, len, off),
                                        yy::missing_loc());  
  case etspecialassign:
    return new SpecialAssign(readCode(buf, len, off), 
                                           readCode(buf, len, off),
                                           yy::missing_loc());  
  case etrequest:
    return new Request(readCode(buf, len, off), 
                                     readCode(buf, len, off),
                                     yy::missing_loc());  
  case ettaggedexpr: {
    E* sym  = readCode(buf, len, off);
    if (sym->etype != etsymbol) {
      throw std::invalid_argument
        (string("readCode: first arg of TaggedExpr must be Symbol, found: ") + 
                            std::to_string(sym->etype));
    }    
    return new TaggedExpr(reinterpret_cast<Symbol*>(sym), 
                                        readCode(buf, len, off),
                                        yy::missing_loc());
  }
  case etarg: {
    bool ref = ntoh<BOOL_T>(*(reinterpret_cast<const BOOL_T*>(buf+off)));
    off += sizeof(BOOL_T);
    return new Arg(ref, readCode(buf, len, off), yy::missing_loc());
  }
  case etfunction: {
    E* el = readCode(buf, len, off);
    if (el->etype != etexprlist) {
      throw std::invalid_argument
        (string("readCode: first arg of Function must be El, found: ") + 
                            std::to_string(el->etype));
    }
    return new Function(reinterpret_cast<El*>(el), 
                                      readCode(buf, len, off),
                                      yy::missing_loc());
  }
  case etfuncall: {
    E* e  = readCode(buf, len, off);
    E* el = readCode(buf, len, off);
    if (el->etype != etexprlist) {
      throw std::invalid_argument
        (string("readCode: second arg of ExprSublist must be El, found: ") + 
                            std::to_string(el->etype));
    } else {
      return new Funcall(e, reinterpret_cast<El*>(el));
    }
  }
  default:
    throw std::domain_error(string("readCode: unknow type ") + 
                            std::to_string(etype));
  }
}


E* zcore::readCode(const char* buf, size_t len) {
#ifdef DEBUG
  cout << "zcore::readCode()" << endl;
  cout << "| buf:" << endl;
  //  cout << printBuf(buf, len) << endl;
#endif
  
  size_t offset = 0;
  E* e = ::readCode(buf, len, offset); // will throw if e can't be read
  if (e == nullptr) {                  // but check just in case
    throw std::logic_error("readCode: nullptr expression");
  }
  return e;
}


static bool isAtomic(val::ValType t) {
  switch (t) {
  case val::vt_std_int:
  // note that the following two are considered atomic even if they actually can
  // be split over multiple buffers: the reason is that the meaning of atomic
  // here is not whether they fit in a buffer, but if they have a sub-element
  // that needs to be read in order to obtain the final element:
  case val::vt_std_string:
  case val::vt_error:
    return true;
  case val::vt_list:
  case val::vt_clos:
  case val::vt_builting:
  case val::vt_string:
  case val::vt_double:
  case val::vt_bool:
  case val::vt_time:
  case val::vt_duration:
  case val::vt_interval:
  case val::vt_period:
  case val::vt_zts:
  case val::vt_future:
  case val::vt_connection:
  case val::vt_named:
    return false;
  default:
    throw std::domain_error(string("encode isAtomic: unknow type '") + 
                            val::vt_to_string.at(t) + '\'');    
  }
} 


static bool isArray(val::ValType t) {
  switch (t) {
  case val::vt_string:
  case val::vt_double:
  case val::vt_bool:
  case val::vt_time:
  case val::vt_duration:
  case val::vt_interval:
  case val::vt_period:
  case val::vt_zts:
    return true;
  default:
    return false;
  }
} 


/// Takes an array list as encoded for the array dimensions and names
/// and builds standard dim and name vectors used in 'Array'
/// constructors.
static void convertFromList(const val::Value& val, 
                            Vector<idx_type>& dim, 
                            vector<Vector<zstring>>& names,
                            val::VI& ordered) 
{
#ifdef DEBUG
  cout << "convertFromList()" << endl;
  cout << "| val: " << endl;
  cout << val::to_string(val) << endl;
#endif
  const auto& lst = get<val::SpVList>(val);

  dim = *get<val::SpVI>(lst->a[0]);

  const auto namesList = get<val::SpVList>(lst->a[1]).get();
  names.resize(namesList->size());
  for (size_t j=0; j<namesList->size(); ++j) {
    const auto subl = get<val::SpVList>(namesList->a[j]);
    for (size_t k=0; k<subl->size(); ++k) {
      names[j].push_back(zstring(get<std::string>(subl->a[k])));
    }
  }

  ordered = *get<val::SpVI>(lst->a[2]);
}


static val::Value make_value(val::ValType vt) {
#ifdef DEBUG
  cout << "make_value()" << endl;
  cout << "| val type: " << val::vt_to_string.at(vt) << endl;
#endif
  static auto idx = arr::Vector<idx_type>{0,1};
  switch (vt) {
  case val::vt_std_int:
    return val::Value(std::make_shared<val::VI>());
  case val::vt_std_string:
    return val::Value(string(""));
  case val::vt_null:
    return val::VNull();
  case val::vt_list:
    return val::Value(make_cow<val::VList>(arr::NOFLAGS));
  case val::vt_double:
    return make_cow<val::VArrayD>(false, val::VArrayD(rsv, idx));
  case val::vt_bool:
    return make_cow<val::VArrayB>(false, val::VArrayB(rsv, idx));
  case val::vt_string:
    return make_cow<val::VArrayS>(false, val::VArrayS(rsv, idx));
  case val::vt_time:
    return make_cow<val::VArrayDT>(false, val::VArrayDT(rsv, idx));
  case val::vt_duration:
    return make_cow<val::VArrayDUR>(false, val::VArrayDUR(rsv, idx));
  case val::vt_interval:
    return make_cow<val::VArrayIVL>(false, val::VArrayIVL(rsv, idx));
  case val::vt_period:
    return make_cow<val::VArrayPRD>(false, val::VArrayPRD(rsv, idx));
  case val::vt_zts:
    return make_cow<arr::zts>(false, arr::zts(val::VArrayDT(rsv, {0}), val::VArrayD(rsv, idx)));
  case val::vt_named:
    return val::VNamed();
  case val::vt_error:
    return val::VError();
  default:
    throw std::domain_error(string("make_value: unknow type ") + std::to_string(vt));
  }
}


template <typename T>
static inline size_t getEltFromBuffer(T& t, const char* buf) {
  t = ntoh<T>(*(reinterpret_cast<const T*>(buf)));
  return sizeof(T);
}

template <>
inline size_t getEltFromBuffer(arr::zstring& t, const char* buf) {
  uint64_t len = ntoh64(*(reinterpret_cast<const uint64_t*>(buf)));
  buf += sizeof(uint64_t);
  const string s(buf, buf + len);
  t = s;
  return sizeof(uint64_t) + getAlignedLength(s.size(), STRALIGN);
}

// bool is special case: we encode it over 8 bytes so sizeof(T) if
template <>
inline size_t getEltFromBuffer(bool& t, const char* buf) {
  t = ntoh<BOOL_T>(*(reinterpret_cast<const BOOL_T*>(buf)));
  return sizeof(BOOL_T);
}


template <typename T>
void readEltsFromBufferSlow(const char* buf, const size_t len, const size_t exp, 
                        size_t& off, Array<T>& a, size_t& n) {
  idx_type col    = n / a.getdim(0);
  idx_type coloff = n % a.getdim(0);
  while (n < exp && off < len) {
    T tmp;                    // to get around the Vector<bool>::reference issue
    off += getEltFromBuffer(tmp, buf+off);
    setv_checkbefore(a.getcol(col), coloff, tmp);
    coloff++;
    if (coloff == a.dim[0]) {
      coloff = 0;
      ++col;
    }
    ++n;
  }
}


// since we don't change order in the case of little endian
// processors, then we can take advantage of memcopy.
#if __BYTE_ORDER == __LITTLE_ENDIAN

template <typename T>
void readEltsFromBuffer(const char* buf, const size_t len, const size_t exp, 
                        size_t& off, Array<T>& a, size_t& n) {
  idx_type col    = n / a.getdim(0);
  idx_type coloff = n % a.getdim(0);
  while (n < exp && off < len) {
    const size_t ncopy = std::min(std::min(exp - n, (len - off) / sizeof(T)), 
                                  a.getdim(0) - coloff);
    const size_t ncopy_bytes = sizeof(T) * ncopy;
    memcpy(a.getcol(col).c_ptr() + coloff, buf + off, ncopy_bytes);
    off += ncopy_bytes;
    n   += ncopy;
    if (coloff + ncopy == a.getdim(0)) {
      coloff = 0;
      ++col;
    }
  }
}

template <>
void readEltsFromBuffer(const char* buf, const size_t len, const size_t exp, 
                        size_t& off, Array<arr::zstring>& a, size_t& n) {
  readEltsFromBufferSlow(buf, len, exp, off, a, n);
}

template <>
void readEltsFromBuffer(const char* buf, const size_t len, const size_t exp, 
                        size_t& off, Array<bool>& a, size_t& n) {
  readEltsFromBufferSlow(buf, len, exp, off, a, n);
}

// in the case where we have to translate to/from little endian to big
// endian, we have the following less efficient routines:
#else

template <>
inline size_t getEltFromBuffer(tz::interval& t, const char* buf) {
  t.s = ntoh<Global::dtime>(*(reinterpret_cast<const Global::dtime*>(buf)));
  t.e = ntoh<Global::dtime>(*(reinterpret_cast<const Global::dtime*>(buf+sizeof(Global::dtime))));
  t.sopen = ntoh<uint32_t>(*(reinterpret_cast<const uint32_t*>(buf+2*sizeof(Global::dtime))));
  t.eopen = ntoh<uint32_t>(*(reinterpret_cast<const uint32_t*>(buf+2*sizeof(Global::dtime) + 
                                                               sizeof(uint32_t))));
  return sizeof(tz::interval);
}


template <>
inline size_t getEltFromBuffer(tz::period& t, const char* buf) {
  t.setMonths(ntoh<int32_t>(*(reinterpret_cast<const int32_t*>(buf))));
  t.setDays(ntoh<int32_t>(*(reinterpret_cast<const int32_t*>(buf+sizeof(int32_t)))));
  t.setDuration(ntoh<Global::duration>(*(reinterpret_cast<const Global::duration*>(buf+sizeof(Global::duration)))));
  return sizeof(tz::period);
}


template <>
inline size_t getEltFromBuffer(bool& t, const char* buf) {
  t = ntoh<BOOL_T>(*(reinterpret_cast<const BOOL_T*>(buf)));
  return sizeof(BOOL_T);
}


template <typename T>
void readEltsFromBuffer(const char* buf, const size_t len, const size_t exp, 
                        size_t& off, Array<T>& a, size_t& n) {
  readEltsFromBufferSlow(buf, len, exp, off, a, n);
}


#endif

/// Read an array from the given buffer 'buf'. An array is a complex
/// value to read as it has multiple stages. State 0 is a list that
/// contains the dimensions and dimension names. Stage 1 is a
/// contiguous array of elements. The buffer may not contain the full
/// array, in which case we read until the end of the buffer.
///
/// \return 0 if a complete array was read (or the array was completed
/// during this invocation), 1 if at the end of the buffer and the
/// array is still incomplete.
template <typename T>
static inline int readArray(zcore::ValState& vs,            ///< current state
                            vector<zcore::ValState>& ss,    ///< state stack
                            size_t& idx,                    ///< state stack index 
                            const char* buf,                ///< buffer
                            size_t len,                     ///< length of buffer
                            size_t& off)                    ///< current offset in buffer
                        
{
#ifdef DEBUG 
  cout << "zcore::readArray() with len: " << len << endl;
  cout << "off:             " << off << endl;
  cout << "ss.size():       " << ss.size() << endl;
  cout << "idx:             " << idx << endl;
  cout << "vs.exp:          " << vs.exp << endl;
  cout << "vs.n:            " << vs.n << endl;
  cout << "ss[idx-1].stage: " << ss[idx-1].stage << endl;
  cout << "buf+off:         " << endl;
  cout << printBuf(buf+off, len-off) << endl;
#endif
  if (ss[idx-1].stage == 0) {
    // idx points to a list of 2 list: (1) list of dims, (2) list
    // of list of names so initialize the array like that
    Vector<idx_type> dim;
    auto names = vector<Vector<zstring>>();
    val::VI ordered;
    convertFromList(ss[idx].val, dim, names, ordered);
    auto a = make_cow<arr::Array<T>>(false, arr::noinit_tag, dim, names);
    vs.val = a;
    for (size_t j=0; j<ordered.size(); ++j) {
      a.get()->getcol(j).setOrdered(ordered[j]);
    }
    ++(ss[idx-1].stage);
    // dont't pop, so we don't read new elements: indeed the next
    // elements are not end coded objects of type 'val::Value', but
    // instead directly a series of 'T'.
  } else if (ss[idx-1].stage == 1) {
    // read the array elements:
    auto& va = *get<arr::cow_ptr<Array<T>>>(vs.val).get(); // don't make a copy!
    readEltsFromBuffer(buf, len, vs.exp, off, va, vs.n);   
    if (off == len && vs.n < vs.exp) return 1;
  } else {
    throw domain_error("invalid stage for array");
  }

  if (vs.n == vs.exp) {
    ss.pop_back();
    --idx;
  }
  return 0;
}


void zcore::readValue(const char* buf, size_t len, size_t& off, vector<ValState>& ss, size_t& idx) {
#ifdef DEBUG
  cout << "zcore::readValue() with len: " << len << endl;
  cout << "| off: " << off << endl;
  cout << "| buf:" << endl;
  cout << printBuf(buf, len) << endl;
#endif

  while (off < len || (idx > 0 && idx < ss.size())) {
    if (idx == ss.size()) {
      val::ValType vt = val::ValType(ntoh64(*(reinterpret_cast<const uint64_t*>(buf+off))));
      off += 8;
#ifdef DEBUG
      cout << "| found:  " << val::vt_to_string.at(vt) << endl;
#endif
      uint64_t vtlen = ntoh64(*(reinterpret_cast<const uint64_t*>(buf+off)));
      off += 8;
      ss.push_back(ValState{make_value(vt), vtlen, 0, 0}); 
      ++idx;

      if (isArray(vt) || (vtlen && !isAtomic(vt))) {
	// for lists, vnamed, etc. where we are expecting multiple
	// elements, we execute the loop again to get an inner element
        // if of course there is such an element (vtlen > 0):
	continue;
      }
    }
    ValState& vs = ss[idx-1];  
#ifdef DEBUG 
    cout << "looping with: " << val::vt_to_string.at(vs.val.which()) << endl;
    cout << "ss.size():    " << ss.size() << endl;
    cout << "idx:          " << idx << endl;
    cout << "stage:        " << ss[idx-1].stage << endl;
    cout << "vs.exp:       " << vs.exp << endl;
    cout << "vs.n:         " << vs.n << endl;
    cout << "buf+off:      " << endl;
    cout << printBuf(buf+off, len-off) << endl;
#endif

    // done processing this buffer if there are still elements to be
    // read but we have read all the given buffer.
    if (off == len && vs.n != vs.exp && idx == ss.size()) {
      return;
    }

    // when we get here, vs points to an unfinished Value 
    switch (vs.val.which()) {

    case val::vt_std_int: {
      auto& vi = get<val::SpVI>(vs.val);
      while (vs.n < vs.exp && off < len) {
        val::VI::value_type tmp;
        off += getEltFromBuffer(tmp, buf+off);
        ++vs.n;
        vi->push_back(tmp);
      }
      if (vs.n >= vs.exp) --idx;
      break;
    }
    case val::vt_std_string: {
      // stage 0, no string created yet
      // stage 1, string created
      size_t readLen = min(vs.exp, len - off);
      // assert len - off is aligned
      const string s(buf+off, buf+off+readLen);
      if (ss[idx-1].stage == 0) {
        vs.val = val::Value(s);
        ++(ss[idx-1].stage);
      }
      else {
        auto& v = get<std::string>(vs.val);
        v += s;
      }
      size_t alen = getAlignedLength(s.size(), STRALIGN);
      off += alen;
      vs.n += alen;
      if (vs.n >= vs.exp) --idx;
      break;
    } 
    case val::vt_error: {
      // stage 0, no string created yet
      // stage 1, string created
      size_t readLen = min(vs.exp, len - off);
      // assert len - off is aligned
      const string s(buf+off, buf+off+readLen);
      if (ss[idx-1].stage == 0) {
        vs.val = val::VError{s};
        ++(ss[idx-1].stage);
      }
      else {
        auto& v = get<val::VError>(vs.val);
        v.what = v.what + s;
      }
      size_t alen = getAlignedLength(s.size(), STRALIGN);
      off += alen;
      vs.n += alen;
      if (vs.n >= vs.exp) --idx;
      break;
    }    
    case val::vt_null:
      --idx;
      break;
    case val::vt_double:
      if (readArray<double>(vs, ss, idx, buf, len, off))
        return;
      else
        break; 
    case val::vt_bool:
      if (readArray<bool>(vs, ss, idx, buf, len, off))
        return;
      else
        break; 
    case val::vt_time:
      if (readArray<Global::dtime>(vs, ss, idx, buf, len, off)) 
        return;
      else
        break; 
    case val::vt_duration:
      if (readArray<Global::duration>(vs, ss, idx, buf, len, off))
        return;
      else
        break; 
    case val::vt_interval:
      if (readArray<tz::interval>(vs, ss, idx, buf, len, off))
        return;
      else
        break; 
    case val::vt_period:
      if (readArray<tz::period>(vs, ss, idx, buf, len, off))
        return;
      else
        break; 
    case val::vt_string:
      if (readArray<arr::zstring>(vs, ss, idx, buf, len, off))
        return;
      else
        break; 
    case val::vt_list: {
      auto& vl = get<val::SpVList>(vs.val);
      if (vs.n < vs.exp) {
        if (idx >= ss.size()) {
          throw std::out_of_range("no element for list");
        }
        if (ss[idx].val.which() == val::vt_named) {
          val::VNamed nv = get<val::VNamed>(ss[idx].val);
          vl->push_back(make_pair(get<std::string>(nv.name), nv.val));
        } 
        else {
          vl->push_back(make_pair(string(""), ss[idx].val));
        }
        ss.pop_back();
        ++vs.n;
      } 
      if (vs.n == vs.exp) --idx;
      break;
    }
    case val::vt_zts: {
      // remember: use get() for non-const instances, so we don't make
      // a copy on dereference!
      if (ss[idx-1].stage == 0) {
        // idx points to an array of double
        auto& a = get<val::SpVAD>(ss[idx].val);
        auto z = make_cow<arr::zts>(false, arr::rsv, a.get()->getdim());
        vs.val = z;
        a.get()->swap(*z.get()->getArrayPtr());
        ++(ss[idx-1].stage);
        ss.pop_back();
        ++vs.n;
        break;
      }
      auto& va = *get<val::SpZts>(vs.val).get(); 
      if (ss[idx-1].stage == 1) {
        // idx points to an array of time
        auto& i = get<val::SpVADT>(ss[idx].val);
        i.get()->swap(*va.getIndexPtr());
        ss.pop_back();
        --idx;
        ++vs.n;
      }
      break; 
    }
    case val::vt_named: {
      auto& vn = get<val::VNamed>(vs.val);
      if (vs.exp != VNAMED_LEN) {
        throw std::domain_error("readValue: invalid VNamed exp");        
      }
      switch (vs.n) {
      case 0: {
        vn.name = ss[idx].val;
        ss.pop_back();
        ++(vs.n);
	break;
      } 
      case 1: {
        vn.val = ss[idx].val;
        ss.pop_back();
        ++(vs.n);
        --idx;
        break;
      }
      default:
        throw std::domain_error("readValue: invalid VNamed n");
      }
      break;
    }
    default:
      throw std::domain_error("readValue: unknow type " + 
                              std::to_string(vs.val.which()));
    }
  }
}


/// Returns the encoded length of the expression 'e'.
size_t zcore::codeLength(const E* e) {
  switch (e->etype) {
  case etempty    : 
  case etnull     : 
  case etbreak    : 
  case etnext     : return sizeof(ETYPE_T);
  case etbool     : return sizeof(ETYPE_T) + sizeof(BOOL_T);
  case etdouble   : return sizeof(ETYPE_T) + sizeof(DOUBLE_T);
  case etdtime    : return sizeof(ETYPE_T) + sizeof(DTIME_T);
  case etinterval : return sizeof(ETYPE_T) + 2*sizeof(DTIME_T) + 2*sizeof(IVL_OPEN_T);
  case etsymbol   : {
    auto s = static_cast<const Symbol*>(e);
    return sizeof(ETYPE_T) + sizeof(STRLEN_T) +
      getAlignedLength(s->data.size(), STRALIGN) + sizeof(BOOL_T);
  }
  case etstring  : {
    auto s = static_cast<const String*>(e);
    return sizeof(ETYPE_T) + sizeof(STRLEN_T) + getAlignedLength((*s->data)[0].size(), STRALIGN);
  }
  case etboundvar: {
    auto bv = static_cast<const Boundvar*>(e);
    return sizeof(ETYPE_T) + sizeof(STRLEN_T) + getAlignedLength(bv->data.size(), STRALIGN);
  }
    // case etconn cannot be sent!
  case etunop : {
    auto u = static_cast<const Unop*>(e);
    return sizeof(ETYPE_T) + sizeof(OP_T) + codeLength(u->e);
  }
  case etbinop : {
    const Binop& b = static_cast<const Binop&>(*e);
    return sizeof(ETYPE_T) + sizeof(OP_T) + codeLength(b.left) + codeLength(b.right);
  }
  case etexprlist : {
    auto& el = static_cast<const El&>(*e);
    ElNode* eln = el.begin;
    unsigned len = sizeof(ETYPE_T) + sizeof(EL_N_T);
    for (unsigned n=0; n<el.n; ++n) {
      len += codeLength(eln->e);
      eln = eln->next;
    }
    return len;
  }
  case etwhile: {
    auto& w = static_cast<const While&>(*e);
    return sizeof(ETYPE_T) + codeLength(w.e1) + codeLength(w.e2);
  }
  case etifelse: {
    auto& i = static_cast<const IfElse&>(*e);
    return sizeof(ETYPE_T) + codeLength(i.e1) + codeLength(i.e2) + codeLength(i.e3);
  }
  case etfor: {
    auto& f = static_cast<const For&>(*e);
    return sizeof(ETYPE_T) + codeLength(f.forloop);
  }
  case etleftassign: {
    auto& la = static_cast<const LeftAssign&>(*e);
    return sizeof(ETYPE_T) + codeLength(la.e1) + codeLength(la.e2);
  }
  case etspecialassign: {
    auto& sa = static_cast<const SpecialAssign&>(*e);
    return sizeof(ETYPE_T) + codeLength(sa.e1) + codeLength(sa.e2);
  }
  case etrequest: {
    auto& r = static_cast<const Request&>(*e);
    return sizeof(ETYPE_T) + codeLength(r.e1) + codeLength(r.e2);
  }
  case ettaggedexpr: {
    auto& et = static_cast<const TaggedExpr&>(*e);
    return sizeof(ETYPE_T) + codeLength(et.symb) + codeLength(et.e);
  }
  case etarg: {
    auto& et = static_cast<const Arg&>(*e);
    return sizeof(ETYPE_T) + sizeof(BOOL_T) + codeLength(et.e);
  }
  case etfunction: {
    auto& f = static_cast<const Function&>(*e);
    return sizeof(ETYPE_T) + codeLength(f.formlist) + codeLength(f.body);
  }
  case etfuncall: {
    auto& es = static_cast<const Funcall&>(*e);
    return sizeof(ETYPE_T) + codeLength(es.e) + codeLength(es.el);
  }
  default:
    throw std::domain_error(string("codeLength: unknow type ") + 
                            std::to_string(e->etype));
  }
}


bool zcore::isTransmissible(const val::Value& v) {
  return !(v.which() == val::vt_builting ||
           v.which() == val::vt_clos ||
           v.which() == val::vt_future ||
           v.which() == val::vt_connection ||
           v.which() == val::vt_timer);
}

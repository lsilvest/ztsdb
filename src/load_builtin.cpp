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


#include "load_builtin.hpp"
#include "base_funcs.hpp"


// #define DEBUG

void core::loadBuiltinFunctions(interp::BaseFrame* r) {
  val::VBuiltinG(r, "all",          "function (...) NULL\n", funcs::all, true);  
  val::VBuiltinG(r, "any",          "function (...) NULL\n", funcs::any, true);  
  val::VBuiltinG(r, "is.nan",       "function (x) NULL\n", funcs::is_nan, true,
                 {{"x", {{val::vt_double, val::vt_zts}, true}}});  
  val::VBuiltinG(r, "is.infinite",  "function (x) NULL\n", funcs::is_infinite, true,
                 {{"x", {{val::vt_double, val::vt_zts}, true}}});  
  val::VBuiltinG(r, "all.equal", "function(x, y) NULL\n", funcs::all_equal, true);

  val::VBuiltinG(r, "as.logical",   "function (x) NULL\n",  funcs::as_logical);
  val::VBuiltinG(r, "as.numeric",   "function (x) NULL\n",  funcs::as_numeric);  
  val::VBuiltinG(r, "as.double",    "function (x) NULL\n",  funcs::as_numeric);
  val::VBuiltinG(r, "as.character", "function (x) NULL\n",  funcs::as_character);  
  val::VBuiltinG(r, "as.duration",  "function (x) NULL\n",  funcs::as_duration);
  val::VBuiltinG(r, "as.period",    "function (x) NULL\n",  funcs::as_period);
  val::VBuiltinG(r, "as.time",      "function (x) NULL\n",  funcs::as_time);
  val::VBuiltinG(r, "as.interval",  "function (x) NULL\n",  funcs::as_interval);
  val::VBuiltinG(r, "period.month", "function (x) NULL\n", 
                 funcs::period_month, false, 
                 {{"x", {{val::vt_period}, true}}});
  val::VBuiltinG(r, "period.day", "function (x) NULL\n", 
                 funcs::period_day, false, 
                 {{"x", {{val::vt_period}, true}}});
  val::VBuiltinG(r, "period.duration", "function (x) NULL\n", 
                 funcs::period_duration, false, 
                 {{"x", {{val::vt_period}, true}}});
  val::VBuiltinG(r, "period", "function (months=0, days=0, duration=as.duration(0)) NULL\n", 
                 funcs::make_period, false, 
                 {{"months",   {{val::vt_double}, true}},
                  {"days",     {{val::vt_double}, true}},
                  {"duration", {{val::vt_duration}, true}}});
  val::VBuiltinG(r, "character",    "function (length=0) NULL\n", funcs::character);  
  val::VBuiltinG(r, "interval",     "function (start, end=NULL, "
                 "duration=NULL, sopen=FALSE, eopen=TRUE) NULL\n", 
                 funcs::_interval, false, 
                 {{"start", {{val::vt_time}, true}},
                  {"end", {{val::vt_time, val::vt_null}, true}},
                  {"duration", {{val::vt_duration, val::vt_null}, true}},
                  {"sopen", {{val::vt_bool}, true}},
                  {"eopen", {{val::vt_bool}, true}}});
  val::VBuiltinG(r, "interval.start", "function (i) NULL\n",
                 funcs::interval_start, false, 
                 {{"i", {{val::vt_interval}, true}}});
  val::VBuiltinG(r, "interval.end", "function (i) NULL\n",
                 funcs::interval_end, false, 
                 {{"i", {{val::vt_interval}, true}}});
  val::VBuiltinG(r, "interval.sopen", "function (i) NULL\n",
                 funcs::interval_sopen, false, 
                 {{"i", {{val::vt_interval}, true}}});
  val::VBuiltinG(r, "interval.eopen", "function (i) NULL\n",
                 funcs::interval_eopen, false, 
                 {{"i", {{val::vt_interval}, true}}});

  val::VBuiltinG(r, "sin", "function (x) NULL\n", funcs::_sin, true, 
                 {{"x", {{val::vt_double, val::vt_zts}, true}}});  
  val::VBuiltinG(r, "sinh", "function (x) NULL\n", funcs::_sinh, false, 
                 {{"x", {{val::vt_double, val::vt_zts}, true}}});  
  val::VBuiltinG(r, "cos", "function (x) NULL\n", funcs::_cos, false, 
                 {{"x", {{val::vt_double, val::vt_zts}, true}}});  
  val::VBuiltinG(r, "cosh", "function (x) NULL\n", funcs::_cosh, false, 
                 {{"x", {{val::vt_double, val::vt_zts}, true}}});  
  val::VBuiltinG(r, "tan", "function (x) NULL\n", funcs::_tan, false, 
                 {{"x", {{val::vt_double, val::vt_zts}, true}}});  
  val::VBuiltinG(r, "tanh", "function (x) NULL\n", funcs::_tanh, false, 
                 {{"x", {{val::vt_double, val::vt_zts}, true}}});  
  val::VBuiltinG(r, "asin", "function (x) NULL\n", funcs::_asin, false, 
                 {{"x", {{val::vt_double, val::vt_zts}, true}}});  
  val::VBuiltinG(r, "asinh", "function (x) NULL\n", funcs::_asinh, false, 
                 {{"x", {{val::vt_double, val::vt_zts}, true}}});  
  val::VBuiltinG(r, "acos", "function (x) NULL\n", funcs::_acos, false, 
                 {{"x", {{val::vt_double, val::vt_zts}, true}}});  
  val::VBuiltinG(r, "acosh", "function (x) NULL\n", funcs::_acosh, false, 
                 {{"x", {{val::vt_double, val::vt_zts}, true}}});  
  val::VBuiltinG(r, "atan", "function (x) NULL\n", funcs::_atan, false, 
                 {{"x", {{val::vt_double, val::vt_zts}, true}}});  
  val::VBuiltinG(r, "atanh", "function (x) NULL\n", funcs::_atanh, false, 
                 {{"x", {{val::vt_double, val::vt_zts}, true}}});  
  val::VBuiltinG(r, "floor", "function (x, unit=NULL, tz=NULL) NULL\n", funcs::_floor, false, 
                 {{"x", {{val::vt_double, val::vt_zts, val::vt_time, val::vt_interval}, true}},
                  {"unit", {{val::vt_string, val::vt_null}, true}},
                  {"tz", {{val::vt_string, val::vt_null}, true}}});  
  val::VBuiltinG(r, "ceiling", "function (x, unit=NULL, tz=NULL) NULL\n", funcs::_ceiling, false, 
                 {{"x", {{val::vt_double, val::vt_zts, val::vt_time, val::vt_interval}, true}},
                  {"unit", {{val::vt_string, val::vt_null}, true}},
                  {"tz", {{val::vt_string, val::vt_null}, true}}});  

  val::VBuiltinG(r, "list", "function(...) NULL\n", funcs::vlist);
  val::VBuiltinG(r, "is.null", "function(x) NULL\n", funcs::is_null);
  val::VBuiltinG(r, "c",  "function(...) NULL\n", funcs::c);
  val::VBuiltinG(r, "do.call", "function(what, args) NULL\n", funcs::do_call);
  val::VBuiltinG(r, "t", "function(x) NULL\n", funcs::t);

  val::VBuiltinG(r, "ls",  "function(name=\"current\") NULL\n", funcs::ls);
  val::VBuiltinG(r, "lsg", "function(name=\"global\") NULL\n", funcs::ls);
  val::VBuiltinG(r, 
                 "get", 
                 "function(x, envir=\"current\", inherits=FALSE) NULL\n", 
                 funcs::getvar, 
                 false,        // don't evaluate args in the ellipsis
                 {{"x",        {{val::vt_string}, true}},
                  {"envir",    {{val::vt_string}, true}},
                  {"inherits", {{val::vt_bool  }, true}}});
  val::VBuiltinG(r, 
                 "assign", 
                 "function(x, value, envir=\"current\", inherits=FALSE) NULL\n", 
                 funcs::assign, 
                 false,        // don't evaluate args in the ellipsis
                 {{"x",        {{val::vt_string}, true}},
                  {"envir",    {{val::vt_string}, true}},
                  {"inherits", {{val::vt_bool  }, true}}});
  val::VBuiltinG(r, 
                 "rm", 
                 "function(..., list=character(), envir=\"current\", inherits=FALSE) NULL\n", 
                 funcs::rm, 
                 false,        // don't evaluate args in the ellipsis
                 {{"list",     {{val::vt_string}, true}},
                  {"envir",    {{val::vt_string}, true}},
                  {"inherits", {{val::vt_bool  }, true}}});
  val::VBuiltinG(r, 
                 "remove", 
                 "function(..., list = character(), envir=\"current\", inherits=FALSE) NULL\n", 
                 funcs::rm, 
                 false,        // don't evaluate args in the ellipsis
                 {{"list",     {{val::vt_string}, true}},
                  {"envir",    {{val::vt_string}, true}},
                  {"inherits", {{val::vt_bool  }, true}}});

  val::VBuiltinG(r, "stats.net", 
                 "function(reset=FALSE) NULL\n", 
                 funcs::stats_net, true,
                 {{"reset",  {{val::vt_bool  }, true}}});
  val::VBuiltinG(r, "stats.msg", 
                 "function(reset=FALSE) NULL\n", 
                 funcs::stats_msg, true,
                 {{"reset",  {{val::vt_bool  }, true}}});
  val::VBuiltinG(r, "stats.ctx", 
                 "function(reset=FALSE) NULL\n", 
                 funcs::stats_ctx, true,
                 {{"reset",  {{val::vt_bool  }, true}}});
  val::VBuiltinG(r, "info.net", "function() NULL\n", funcs::info_net);
  val::VBuiltinG(r, "info.msg", "function() NULL\n", funcs::info_msg);
  val::VBuiltinG(r, "info.ctx", "function() NULL\n", funcs::info_ctx);
   
  val::VBuiltinG(r, "length", "function(x) NULL\n", funcs::length);
  val::VBuiltinG(r,
                 "matrix",
                 "function(data = NA, nrow = 1, ncol = 1, byrow = FALSE, "
                 "dimnames = NULL, file=\"\") NULL\n",
                 funcs::make_matrix, true,
                 {{"nrow",     {{val::vt_double}, true}},
                  {"ncol",     {{val::vt_double}, true}},
                  {"byrow",    {{val::vt_bool  },  true}},
                  {"dimnames", {{val::vt_null, val::vt_list}, true}},
                  {"file",     {{val::vt_string}, true}}});
  val::VBuiltinG(r,
                 "array",
                 "function(data = NA, dim = length(data), "
                 "dimnames = NULL, file=\"\") NULL\n",
                 funcs::make_array, true,
                 {{"dim",      {{val::vt_double}, true}},
                  {"dimnames", {{val::vt_null, val::vt_list  }, true}},
                  {"file",     {{val::vt_string}, true}}});
  val::VBuiltinG(r,
                 "vector",
                 "function(mode=\"logical\", length=0, file=\"\") NULL\n",
                 funcs::make_vector, true,
                 {{"mode",   {{val::vt_string }, true}},
                  {"length", {{val::vt_double }, true}},
                  {"file",   {{val::vt_string }, true}}});
  val::VBuiltinG(r, "dim", "function(x) NULL\n", funcs::dim);
  val::VBuiltinG(r, "ncol", "function(x) NULL\n", funcs::ncol);
  val::VBuiltinG(r, "nrow", "function(x) NULL\n", funcs::nrow);

  val::VBuiltinG(r, "dimnames", "function(x, ...) NULL\n", funcs::dimnames);
  val::VBuiltinG(r, "colnames", "function(x, ...) NULL\n", funcs::colnames);
  val::VBuiltinG(r, "rownames", "function(x, ...) NULL\n", funcs::rownames);

  val::VBuiltinG(r, "zts.resize", "function(x, start=1, end=nrow(x)) NULL\n", 
                 funcs::zts_resize, true,
                 {{"x"    , {{val::vt_zts}, true}},
                  {"start", {{val::vt_double,val::vt_time}, true}},
                  {"end",   {{val::vt_double,val::vt_time}, true}}});
  val::VBuiltinG(r, "zts.truncate", "function(x, end) NULL\n", 
                 funcs::zts_truncate, true,
                 {{"x"    , {{val::vt_zts}, true}},
                  {"end",   {{val::vt_double,val::vt_time}, true}}});

  val::VBuiltinG(r, "rbind", "function(...) NULL\n", funcs::rbind);
  val::VBuiltinG(r, "cbind", "function(...) NULL\n", funcs::cbind);
  val::VBuiltinG(r, "abind", "function(..., along) NULL\n", funcs::abind);
  val::VBuiltinG(r, "connection", "function(ip, port) NULL\n", funcs::make_connection, true,
                 {{"ip",   {{val::vt_string}, true}},
                  {"port", {{val::vt_double}, true}}});
  val::VBuiltinG(r, "connection.port", "function(connection) NULL\n", funcs::connection_port, true,
                 {{"connection",   {{val::vt_connection}, true}}});
  val::VBuiltinG(r, "connection.address", "function(connection) NULL\n", 
                 funcs::connection_address, true,
                 {{"connection",   {{val::vt_connection}, true}}});
  val::VBuiltinG(r, "timer", "function(duration, loop, once=NULL, loop_max=0) NULL\n", 
                 funcs::make_timer, true,
                 {{"duration", {{val::vt_duration}, true}},
                  {"loop",     {{val::vt_string},   false}},
                  {"once",     {{val::vt_string},   false}},
                  {"loop_max", {{val::vt_double},   true}}});
  val::VBuiltinG(r,
                 "zts",
                 "function(idx, data = NA, "
                 "dim = if (length(dim(data)) > 1) dim(data) "
                 "      else c(length(idx), length(data) / length(idx)), "
                 "dimnames = NULL, file=\"\") NULL\n",
                 funcs::make_zts, true,
                 {{"idx",      {{val::vt_time              }, true}},
                  {"data",     {{val::vt_double            }, true}},
                  {"dim",      {{val::vt_double            }, true}},
                  {"dimnames", {{val::vt_null, val::vt_list}, true}},
                  {"file",     {{val::vt_string            }, true}}});
  val::VBuiltinG(r,
                 "zts.idx",
                 "function(zts) NULL\n",
                 funcs::zts_idx, true,
                 {{"zts",      {{val::vt_zts               }, true}}});
  val::VBuiltinG(r,
                 "zts.data",
                 "function(zts) NULL\n",
                 funcs::zts_data, true,
                 {{"zts",      {{val::vt_zts               }, true}}});
  val::VBuiltinG(r, "head", "function(x, n=6) NULL\n", funcs::head, true,
                 {{"x",  {{val::vt_double, val::vt_bool, val::vt_string, val::vt_time, 
                           val::vt_duration, val::vt_interval, val::vt_list, val::vt_zts}, true}},
                  {"n",  {{val::vt_double}, true}}});
  val::VBuiltinG(r, "tail", "function(x, n=6, addrownums=TRUE) NULL\n", funcs::tail, true,
                 {{"x",  {{val::vt_double, val::vt_bool, val::vt_string, val::vt_time, 
                           val::vt_duration, val::vt_interval, val::vt_list, val::vt_zts}, true}},
                  {"n",  {{val::vt_double}, true}},
                  {"addrownums",  {{val::vt_bool}, true}}});
  val::VBuiltinG(r, "runif", "function(x, min=0, max=1) NULL\n", funcs::runif, true,
                 {{"x",   {{val::vt_double, val::vt_zts}, true}},
                  {"min", {{val::vt_double}, true}},
                  {"max", {{val::vt_double}, true}}});
  val::VBuiltinG(r, "alloc.dirname", "function(x) NULL\n", funcs::alloc_dirname, true); 
  val::VBuiltinG(r, "msync", "function(x, async=FALSE) NULL\n", funcs::msync, true,
                 {{"async", {{val::vt_bool}, true}}});

  val::VBuiltinG(r, "typeof", "function(x) NULL\n", funcs::get_typeof, false);
  val::VBuiltinG(r,
                 "dyn.load", 
                 "function (x, local = TRUE, now = TRUE) NULL\n",
                 funcs::dyn_load, true,
                 {{"local", {{val::vt_bool}, true}},
                  {"now",   {{val::vt_bool}, true}}});
  val::VBuiltinG(r, "options", "function(...) NULL\n", funcs::_options);

  val::VBuiltinG(r, "subset", "function(..., drop = TRUE) NULL\n", funcs::subset, true,
                 {{"drop", {{val::vt_bool}, true}}});
  val::VBuiltinG(r, "dblsubset", "function(...) NULL\n", funcs::dblsubset);
  val::VBuiltinG(r, "subassign", "function(...) NULL\n", funcs::subassign);
  val::VBuiltinG(r, "dblsubassign", "function(...) NULL\n", funcs::dblsubassign);
  val::VBuiltinG(r, "load", "function(file) NULL\n", funcs::load, true,
                 {{"file", {{val::vt_string}, true}}});
  
  val::VBuiltinG(r,
                "sort",
                 "function(x, decreasing=FALSE, set=FALSE) NULL\n",
                 funcs::_sort, true,
                 {{"decreasing", {{val::vt_bool }, true}},
                  {"set",        {{val::vt_bool }, true}}});
  val::VBuiltinG(r,
                "sort.idx",
                 "function(x, decreasing=FALSE) NULL\n",
                 funcs::_sort_idx, true,
                 {{"decreasing", {{val::vt_bool }, true}}});
  val::VBuiltinG(r,
                "is.ordered",
                 "function(x) NULL\n",
                 funcs::is_ordered, true);
  

  val::VBuiltinG(r, "cat", "function(..., file = \"\", sep = \" \", fill = FALSE, labels = NULL,"
                 "append = FALSE) NULL\n", funcs::cat, true,
                 {{"file",   {{val::vt_string               }, true}},
                  {"sep",    {{val::vt_string               }, true}},
                  {"fill",   {{val::vt_bool, val::vt_double }, true}},
                  {"labels", {{val::vt_null,val::vt_string  }, true}}});
  val::VBuiltinG(r, "paste",  "function(..., sep = \" \", collapse = NULL) NULL\n", 
                 funcs::paste, true,
                 {{"sep",      {{val::vt_string             }, true}},
                  {"collapse", {{val::vt_null,val::vt_string}, true}}});
  val::VBuiltinG(r, "paste0", "function(..., sep = \"\",  collapse = NULL) NULL\n", 
                 funcs::paste, true,
                 {{"sep",      {{val::vt_string             }, true}},
                  {"collapse", {{val::vt_null,val::vt_string}, true}}});
  val::VBuiltinG(r, "print","function(x, tz=\"\") NULL\n", funcs::print);
  val::VBuiltinG(r, "str", "function(object) NULL\n", funcs::str, true);

  val::VBuiltinG(r, "substr", "function(x, start, stop) NULL \n", funcs::substr, true,
                 {{"x",     {{val::vt_string }, true}},
                  {"start", {{val::vt_double }, true}},
                  {"stop",  {{val::vt_double }, true}}});
  val::VBuiltinG(r, "tryCatch", "function(expr, catch) NULL\n", funcs::tryCatch, false,
                 {{"expr",  {{val::vt_string }, false}},    // don't eval
                  {"catch", {{val::vt_double }, false}}});  // don't eval
  val::VBuiltinG(r,
                 "read.csv",
                 "function(csvfile, type=\"zts\", "
                 "arrayfile=\"\", header=TRUE, sep=\",\", tz=\"\") NULL\n",
                 funcs::read_csv, true,
                 {{"csvfile",   {{val::vt_string }, true}},
                  {"type",      {{val::vt_string }, true}},
                  {"arrayfile", {{val::vt_string }, true}},
                  {"header",    {{val::vt_bool   }, true}},
                  {"sep",       {{val::vt_string }, true}},
                  {"tz",        {{val::vt_string }, true}}});
  val::VBuiltinG(r,
                 "write.csv",
                 "function(object, file, header=TRUE, sep=\",\") NULL\n",
                 funcs::write_csv, true,
                 {{"file",      {{val::vt_string }, true}},
                  {"header",    {{val::vt_bool   }, true}},
                  {"sep",       {{val::vt_string }, true}}});


  val::VBuiltinG(r, "rollmean", 
                 "function(x, window, nvalid=window) NULL \n", 
                 funcs::rollmean, true,
                 {{"x",      {{val::vt_double, val::vt_zts }, true}},
                  {"window", {{val::vt_double              }, true}},
                  {"nvalid", {{val::vt_double              }, true}}});
  val::VBuiltinG(r, "rollmin", 
                 "function(x, window, nvalid=window) NULL \n", 
                 funcs::rollmin, true,
                 {{"x",      {{val::vt_double, val::vt_zts }, true}},
                  {"window", {{val::vt_double              }, true}},
                  {"nvalid", {{val::vt_double              }, true}}});
  val::VBuiltinG(r, "rollmax", 
                 "function(x, window, nvalid=window) NULL \n", 
                 funcs::rollmax, true,
                 {{"x",      {{val::vt_double, val::vt_zts }, true}},
                  {"window", {{val::vt_double              }, true}},
                  {"nvalid", {{val::vt_double              }, true}}});
  val::VBuiltinG(r, "rollvar", 
                 "function(x, window, nvalid=window) NULL \n", 
                 funcs::rollvar, true,
                 {{"x",      {{val::vt_double, val::vt_zts }, true}},
                  {"window", {{val::vt_double              }, true}},
                  {"nvalid", {{val::vt_double              }, true}}});
  val::VBuiltinG(r, "rollcov", 
                 "function(x, y, window, nvalid=window) NULL \n", 
                 funcs::rollcov, true,
                 {{"x",      {{val::vt_double, val::vt_zts }, true}},
                  {"y",      {{val::vt_double, val::vt_zts }, true}},   
                  {"window", {{val::vt_double              }, true}},
                  {"nvalid", {{val::vt_double              }, true}}});
  val::VBuiltinG(r, "locf", 
                 "function(x, n) NULL \n", 
                 funcs::locf, true,
                 {{"x", {{val::vt_double, val::vt_zts }, true}},
                  {"n", {{val::vt_double },              true}}});
  val::VBuiltinG(r, "move", 
                 "function(x, n) NULL \n", 
                 funcs::move, true,
                 {{"x", {{val::vt_double, val::vt_zts }, true}},
                  {"n", {{val::vt_double              }, true}}});
  val::VBuiltinG(r, "rotate", 
                 "function(x, n) NULL \n", 
                 funcs::rotate, true,
                 {{"x", {{val::vt_double, val::vt_zts }, true}},
                  {"n", {{val::vt_double              }, true}}});
  val::VBuiltinG(r, "cumsum", 
                 "function(x, rev=FALSE) NULL \n", 
                 funcs::cumsum, true,
                 {{"x",   {{val::vt_double, val::vt_zts }, true}},
                  {"rev", {{val::vt_bool                }, true}}});
  val::VBuiltinG(r, "cumprod", 
                 "function(x, rev=FALSE) NULL \n", 
                 funcs::cumprod, true,
                 {{"x",   {{val::vt_double, val::vt_zts }, true}},
                  {"rev", {{val::vt_bool                }, true}}});
  val::VBuiltinG(r, "cumdiv", 
                 "function(x, rev=FALSE) NULL \n", 
                 funcs::cumdiv, true,
                 {{"x",   {{val::vt_double, val::vt_zts }, true}},
                  {"rev", {{val::vt_bool                }, true}}});
  val::VBuiltinG(r, "cummax", 
                 "function(x, rev=FALSE) NULL \n", 
                 funcs::cummax, true,
                 {{"x",   {{val::vt_double, val::vt_zts }, true}},
                  {"rev", {{val::vt_bool                }, true}}});
  val::VBuiltinG(r, "cummin", 
                 "function(x, rev=FALSE) NULL \n", 
                 funcs::cummin, true,
                 {{"x",   {{val::vt_double, val::vt_zts }, true}},
                  {"rev", {{val::vt_bool                }, true}}});
  val::VBuiltinG(r, "rev", "function(x) NULL \n", funcs::rev, true);
  val::VBuiltinG(r, "sum", "function(x) NULL \n", funcs::sum, true,
                 {{"x",   {{val::vt_double, val::vt_duration, val::vt_zts }, true}}});
  val::VBuiltinG(r, "prod", "function(x) NULL \n", funcs::prod, true,
                 {{"x",   {{val::vt_double, val::vt_zts }, true}}});

  val::VBuiltinG(r, "q",    "function(status=0) NULL\n", funcs::quit, false,
                 {{"status", {{val::vt_double}, true}}});
  val::VBuiltinG(r, "quit", "function(status=0) NULL\n", funcs::quit, false,
                 {{"status", {{val::vt_double}, true}}});
  val::VBuiltinG(r, "stop", "function(x) NULL\n", funcs::stop, false,
                 {{"x", {{val::vt_string}, true}}});
  val::VBuiltinG(r, "source", "function(file) NULL\n", funcs::source, true,
                 {{"file", {{val::vt_string}, true}}});
  val::VBuiltinG(r, "system", 
                 "function(command, intern=FALSE, ignore.stdout=FALSE," 
                 "ignore.stderr=FALSE, wait=TRUE) NULL \n", 
                 funcs::system, true,
                 {{"command",       {{val::vt_string }, true}},
                  {"intern",        {{val::vt_bool   }, true}},
                  {"ignore.stdout", {{val::vt_bool   }, true}},
                  {"ignore.stderr", {{val::vt_bool   }, true}},
                  {"wait",          {{val::vt_bool   }, true}}});
  val::VBuiltinG(r, "Sys.time", "function() NULL \n", funcs::sys_time, true);

  val::VBuiltinG(r, "seq",
                 "function(from=1, to=NULL, by=NULL, length.out=NULL, tz=NULL) NULL \n", 
                 funcs::seq, true,
                 {{"from",  {{val::vt_double,val::vt_time,val::vt_interval,val::vt_null }, true}},
                  {"to",    {{val::vt_double,val::vt_time,val::vt_interval,val::vt_null }, true}},
                  {"by",    {{val::vt_double,val::vt_duration,val::vt_period,val::vt_null }, true}},
                  {"length.out", {{val::vt_double,val::vt_null }, true}},
                  {"tz", {{val::vt_string,val::vt_null }, true}}});

  /// set functions
  val::VBuiltinG(r, "intersect",
		 "function(x, y) NULL \n", 
		 funcs::intersect, true,
		 {{"x", {{val::vt_double,val::vt_time,val::vt_interval,val::vt_duration,
                           val::vt_string,val::vt_bool}, true}},
                  {"y", {{val::vt_double,val::vt_time,val::vt_interval,val::vt_duration,
                          val::vt_string,val::vt_bool}, true}}});
  val::VBuiltinG(r, "union",
		 "function(x, y) NULL \n", 
		 funcs::_union, true,
		 {{"x", {{val::vt_double,val::vt_time,val::vt_interval,val::vt_duration,
                           val::vt_string,val::vt_bool}, true}},
                  {"y", {{val::vt_double,val::vt_time,val::vt_interval,val::vt_duration,
                          val::vt_string,val::vt_bool}, true}}});
  val::VBuiltinG(r, "setdiff",
		 "function(x, y) NULL \n", 
		 funcs::setdiff, true,
		 {{"x", {{val::vt_double,val::vt_time,val::vt_interval,val::vt_duration,
                          val::vt_string,val::vt_bool}, true}},
                  {"y", {{val::vt_double,val::vt_time,val::vt_interval,val::vt_duration,
                          val::vt_string,val::vt_bool}, true}}});
  val::VBuiltinG(r, "intersect.idx",
		 "function(x, y) NULL \n", 
		 funcs::intersect_idx, true,
		 {{"x", {{val::vt_double,val::vt_time,val::vt_interval,val::vt_duration,
                           val::vt_string,val::vt_bool}, true}},
                  {"y", {{val::vt_double,val::vt_time,val::vt_interval,val::vt_duration,
                          val::vt_string,val::vt_bool}, true}}});
  val::VBuiltinG(r, "union.idx",
        	 "function(x, y) NULL \n", 
        	 funcs::union_idx, true,
        	 {{"x", {{val::vt_double,val::vt_time,val::vt_interval,val::vt_duration,
                           val::vt_string,val::vt_bool}, true}},
                  {"y", {{val::vt_double,val::vt_time,val::vt_interval,val::vt_duration,
                          val::vt_string,val::vt_bool}, true}}});
  val::VBuiltinG(r, "setdiff.idx",
        	 "function(x, y) NULL \n", 
        	 funcs::setdiff_idx, true,
        	 {{"x", {{val::vt_double,val::vt_time,val::vt_interval,val::vt_duration,
                           val::vt_string,val::vt_bool}, true}},
                  {"y", {{val::vt_double,val::vt_time,val::vt_interval,val::vt_duration,
                          val::vt_string,val::vt_bool}, true}}});

  // align.idx
  // align vector align, zts align
  val::VBuiltinG(r, "align",
        	 "function(from, to, start=as.duration(0), end=as.duration(0), "
                 "method=\"closest\", tz=NULL) NULL \n", 
        	 funcs::align, true,
        	 {{"from",   {{val::vt_zts},  true}},
                  {"to",     {{val::vt_time}, true}},
                  {"start",  {{val::vt_duration,val::vt_period}, true}},
                  {"end",    {{val::vt_duration,val::vt_period}, true}},
                  {"method", {{val::vt_string}, true}},
                  {"tz",     {{val::vt_string,val::vt_null }, true}}});
  val::VBuiltinG(r, "align.idx",
        	 "function(from, to, start=as.duration(0), end=as.duration(0), "
                 "tz=NULL) NULL \n", 
        	 funcs::align_idx, true,
        	 {{"from",   {{val::vt_time},  true}},
                  {"to",     {{val::vt_time}, true}},
                  {"start",  {{val::vt_duration,val::vt_period}, true}},
                  {"end",    {{val::vt_duration,val::vt_period}, true}},
                  {"tz",     {{val::vt_string,val::vt_null }, true}}});
  val::VBuiltinG(r, "op.zts",
        	 "function(x, y, method) NULL \n", 
        	 funcs::op_zts, true,
        	 {{"x",      {{val::vt_zts}, true}},
                  {"y",      {{val::vt_zts}, true}},
                  {"method", {{val::vt_string}, true}}});
  
  val::VBuiltinG(r, "time",
        	 "function(year, month, day, hour=NULL, min=NULL, sec=NULL, nsec=NULL, tz) NULL \n", 
        	 funcs::make_time, true,
        	 {{"year",  {{val::vt_double}, true}},
                  {"month", {{val::vt_double}, true}},
                  {"day",   {{val::vt_double}, true}},
                  {"hour",  {{val::vt_double,val::vt_null}, true}},
                  {"min",   {{val::vt_double,val::vt_null}, true}},
                  {"sec",   {{val::vt_double,val::vt_null}, true}},
                  {"nsec",  {{val::vt_double,val::vt_null}, true}},
                  {"tz",    {{val::vt_string}, true}}});

  val::VBuiltinG(r, "dayweek", "function(x, tz) NULL\n", funcs::dayweek, true, 
                 {{"x",  {{val::vt_time}, true}},  
                  {"tz", {{val::vt_string }, true}}});
  val::VBuiltinG(r, "daymonth", "function(x, tz) NULL\n", funcs::daymonth, true, 
                 {{"x",  {{val::vt_time}, true}},
                  {"tz", {{val::vt_string }, true}}});
  val::VBuiltinG(r, "month", "function(x, tz) NULL\n", funcs::month, true, 
                 {{"x",  {{val::vt_time}, true}},  
                  {"tz", {{val::vt_string }, true}}});
  val::VBuiltinG(r, "year", "function(x, tz) NULL\n", funcs::year, true, 
                 {{"x",  {{val::vt_time}, true}},
                  {"tz", {{val::vt_string }, true}}});
}


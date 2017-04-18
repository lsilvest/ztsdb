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


#include <iostream>
#include <fstream>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <boost/filesystem.hpp>
#include <sys/eventfd.h>
#include "timezone/zone.hpp"
#include "load_builtin.hpp"
#include "cmdline.h"            // generated by gengetopt
#include "anf.hpp"
#include "parser_ctx.hpp"
#include "logging.hpp"
#include "config.hpp"
#include "config_ctx.hpp"
#include "msg_handler.hpp"


namespace fsys = boost::filesystem;


// #define DEBUG

// definition of externs:
shared_ptr<interp::BaseFrame> base = make_shared<interp::Frame>("base"s);
shared_ptr<interp::BaseFrame> global = make_shared<interp::Frame>("global"s, nullptr, base);
tz::Zones tzones;
zlog::Logger lg;
cfg::CfgMap cfg::cfgmap;


static void* executeNetHandler(void* args_p) {
  auto args = static_cast<std::pair<net::NetHandler&, volatile bool&>*>(args_p);
  auto& c = static_cast<net::NetHandler&>(args->first);
  try {
    c.run(args->second);
  }
  catch (std::exception& e) {
    std::cout << "executeNetHandler exception is: " << e.what() << std::endl;
    lg.log(zlog::SV_ERROR, e.what());
    throw;
  }

  return nullptr;
}



/// Replaces the values in 'cfgmap' but the NetHandlerand line options (if
/// present). NetHandlerand line options take precedence over the ones
/// specified in the config file or the default ones. We actually
/// modify the 'cfgmap' with these new values, so that a user can see
/// what configuration the program is actually running with.
static void supersedeWithCmdLine(const gengetopt_args_info& args_info) 
{
  // if we're only evaluation one expression, then set the listen
  // port to 0 since we won't serve incoming connections:
  if (args_info.eval_mode_counter) {
    cfg::cfgmap.set("port", 0L);
  }    
  else if (args_info.port_given) {
    cfg::cfgmap.set("port", static_cast<int64_t>(args_info.port_arg));
  }
  
  if (args_info.address_given) {
    cfg::cfgmap.set("address", std::string(args_info.address_arg));
  }   

  if (args_info.log_level_given) {
    switch (args_info.log_level_arg) {
    case log_level_arg_TRACE:
      cfg::cfgmap.set("log.level", "TRACE");
    case log_level_arg_DEBUG:
      cfg::cfgmap.set("log.level", "DEBUG");
    case log_level_arg_INFO:
      cfg::cfgmap.set("log.level", "INFO");
    case log_level_arg_ERROR:
      cfg::cfgmap.set("log.level", "ERROR");
      break;
    default:
      // should never happen as args_info should only allow the above
      // values. But it's not important enough to warrant erroring out
      // here, so just print a message and go on:
      std::cerr  << "invalid log level" << std::endl;
    }
  }   
}


std::string getlogfile(std::string progname) {
  // if a logfile is specified, use it:
  fsys::path logfile_name = get<std::string>(cfg::cfgmap.get("logfile.name"));
  fsys::path logfile_path = get<std::string>(cfg::cfgmap.get("logfile.path"));
  fsys::path logfile;
  if (strlen(logfile_name.c_str())) {
    logfile = logfile_path / logfile_name;
  }
  // else use default + process number
  else {
    logfile = logfile_path / 
      (progname + "-"s + std::to_string(getpid()) + Global::LOGFILE_EXTENSION);
  }
  return logfile.c_str();
}


void defineConstants(interp::BaseFrame* r) {
  r->add("version"s, val::make_array(arr::zstring(CMDLINE_PARSER_VERSION)));
}


/// À la mémoire de Pieter Hintjens.
int main(int argc, char *argv[]) {
  gengetopt_args_info args_info;
  if (cmdline_parser(argc, argv, &args_info) != 0) { // function generated by gengetopt
    return EXIT_FAILURE;
  }

  const std::string initcode = args_info.init_code_given ? args_info.init_code_arg + "\n"s : ""s;

  if (args_info.eval_mode_counter > 0 || !(args_info.help_given || args_info.version_given)) {
    // parse and configure options from config file; this is a special
    // case, as of course we can't bite our tail and get the config
    // file path from the config file; so here we allow getting the
    // config file from an environment variable, but priority is still
    // given to the command line:
    const char* dirname_env = getenv(Global::ZTSDBDIR_ENV);
    if (!dirname_env) dirname_env = "";
    fsys::path configfile = fsys::absolute((args_info.config_path_given ? 
                                            args_info.config_path_arg : dirname_env)
                                           + string(Global::CONFIG_FILENAME));
    try {
      cfg::read(cfg::cfgmap, configfile);
    }
    catch (cfg::CfgCtx::NoConfigFile& e) {
      if (args_info.config_path_given) {
        cerr << "can't find config file: " << configfile << std::endl;
        cmdline_parser_free(&args_info);
        return EXIT_FAILURE;
      }
      // otherwise it's OK: there were no config files specified and
      // we will take defaults
    }
    catch (std::exception& e) {
      cerr << "config file error: " << e.what() << std::endl;
      cmdline_parser_free(&args_info);
      return EXIT_FAILURE;
    }

    supersedeWithCmdLine(args_info);


    try {
      tzones.init(get<std::string>(cfg::cfgmap.get("timezone.path")));
    }
    catch (std::exception& e) {
      cerr << "time zone initialization error: " << e.what() << std::endl;
      cmdline_parser_free(&args_info);
      return EXIT_FAILURE;
    }

    try {
      // enable logging:
      lg.init(getlogfile(argv[0]), get<std::string>(cfg::cfgmap.get("timezone")));
      lg.setLevel(zlog::from_string(get<std::string>(cfg::cfgmap.get("log.level"))));
      lg.log(zlog::SV_INFO, "ztsdb started");

      auto lport = static_cast<int>(get<int64_t>(cfg::cfgmap.get("port")));
      auto address = get<std::string>(cfg::cfgmap.get("address"));

      // create the two eventfd we need in order to communicate data and
      // signalling from 'com' to 'interp_run':
      int data_com_ir = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
      if (data_com_ir == -1) {
        throw std::system_error(std::error_code(errno, std::system_category()), 
                                "eventfd failed for data_com_ir");
      }
      int sig_com_ir = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
      if (sig_com_ir == -1) {
        throw std::system_error(std::error_code(errno, std::system_category()), 
                                "eventfd failed for sig_com_ir");
      }
    
      net::NetHandler com(address, lport, data_com_ir, sig_com_ir);

      // load predefined functions in global env:
      core::loadBuiltinFunctions(base.get());
      defineConstants(base.get());
      
      // set the signal mask before running the comm thread:
      sigset_t set;
      int s;
      sigemptyset(&set);
      sigaddset(&set, SIGINT);
      sigaddset(&set, SIGHUP);
      sigaddset(&set, SIGSTOP);
      sigaddset(&set, SIGTSTP);
      sigaddset(&set, SIGTTIN);
      sigaddset(&set, SIGTTOU);
      s = pthread_sigmask(SIG_BLOCK, &set, NULL);
      if (s != 0) {
        throw std::system_error(std::error_code(errno, std::system_category()), "pthread_sigmask");
      }

      // run the TCP comm thread:
      volatile bool stop = 0;
      auto args = std::pair<net::NetHandler&, volatile bool&>{com, stop};
      pthread_t t1;
      pthread_create(&t1, NULL, executeNetHandler, &args); 
    
      // running in server mode--------------------
      if (!args_info.eval_mode_counter) {
        cmdline_parser_print_version();

        zcore::MsgHandler ir(com, global, data_com_ir, sig_com_ir, STDIN_FILENO, false, initcode);
        try {
          ir.run();           // run the interpreter
        }
        catch (const Global::QuitException& e) {
          stop = true;
          pthread_join(t1, nullptr);
          lg.log(zlog::SV_INFO, "ztsdb quit");
          cmdline_parser_free(&args_info);
          return e.status;
        }        
      }
      // running in eval mode--------------------
      else {                      // args_info.eval_mode_counter > 0
        string query = args_info.expression_arg + "\n"s;

        // create a pipe from this thread to ir in order to give it the
        // query, which we will pass in lieu of stdin:
        int pipe_here_ir[2];
        if (pipe2(pipe_here_ir, O_CLOEXEC) == -1) {
          throw std::system_error(std::error_code(errno, std::system_category()), 
                                  "pipe2 here_ir");
        }
        cfg::cfgmap.set("prompt", ""s); // disable prompt
        zcore::MsgHandler ir(com, global, data_com_ir, sig_com_ir, pipe_here_ir[0], true, initcode);
        if (write(pipe_here_ir[1], query.c_str(), query.size()) == -1) {
          throw std::system_error(std::error_code(errno, std::system_category()),
                                  "write pipe_here_ir");
        }
        auto retval = EXIT_SUCCESS;
        try {
          ir.run();
        }
        catch (const Global::QuitException& e) {
          retval = e.status;
        }
        stop = true;
        pthread_join(t1, nullptr);
        cmdline_parser_free(&args_info);
        return retval;
      }
    }
    catch (std::exception& e) {
      std::cerr << e.what() << std::endl;
      lg.log(zlog::SV_ERROR, e.what());
      cmdline_parser_free(&args_info);
      return EXIT_FAILURE;
    }
  }
  else if (!(args_info.help_given || args_info.version_given)) {
    std::cerr << argv[0] << ": invalid options" << std::endl;
    cmdline_parser_free(&args_info);
    return EXIT_FAILURE;
  }
}

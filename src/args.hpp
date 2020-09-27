//
// App:         WeatherFlow Tempest UDP Relay 
// Author:      Mirco Caramori
// Copyright:   (c) 2020 Mirco Caramori
// Repository:  https://github.com/mircolino/tempest
//
// Description: class to handle command line arguments and options
//

#ifndef TEMPEST_ARGUMENTS
#define TEMPEST_ARGUMENTS

// Includes -------------------------------------------------------------------------------------------------------------------

#include "system.hpp"

// Source ---------------------------------------------------------------------------------------------------------------------

// Argument presence

#define TEMPEST_ARG_URL         0b0000000000000001
#define TEMPEST_ARG_FORMAT      0b0000000000000010
#define TEMPEST_ARG_INTERVAL    0b0000000000000100
#define TEMPEST_ARG_LOG         0b0000000000001000
#define TEMPEST_ARG_DAEMON      0b0000000000010000
#define TEMPEST_ARG_TRACE       0b0000000000100000
#define TEMPEST_ARG_STOP        0b0000000001000000
#define TEMPEST_ARG_VERSION     0b0000000010000000
#define TEMPEST_ARG_HELP        0b0000000100000000

#define TEMPEST_ARG_EMPTY       0b0100000000000000
#define TEMPEST_ARG_INVALID     0b1000000000000000

// Mask to validate the presence of all required argument(s) that make a specific command valid
// Expand to TRUE if all required arguments are present

#define TEMPEST_REQ_RELAY(c)    ((c & TEMPEST_ARG_URL) == TEMPEST_ARG_URL)
#define TEMPEST_REQ_TRACE(c)    ((c & TEMPEST_ARG_TRACE) == TEMPEST_ARG_TRACE)
#define TEMPEST_REQ_STOP(c)     ((c & TEMPEST_ARG_STOP) == TEMPEST_ARG_STOP)
#define TEMPEST_REQ_VERSION(c)  ((c & TEMPEST_ARG_VERSION) == TEMPEST_ARG_VERSION)
#define TEMPEST_REQ_HELP(c)     ((c & TEMPEST_ARG_HELP) == TEMPEST_ARG_HELP)

#define TEMPEST_UDP_TRACE(c)    ((c & (TEMPEST_ARG_TRACE | TEMPEST_ARG_FORMAT | TEMPEST_ARG_INTERVAL)) == TEMPEST_ARG_TRACE)

// Mask to validate the presence of only required and optional argument(s) that make a specific command valid
// Expand to TRUE if not only required and optional arguments are present

#define TEMPEST_INV_RELAY(c)    (c & ~(TEMPEST_ARG_URL | TEMPEST_ARG_FORMAT | TEMPEST_ARG_INTERVAL | TEMPEST_ARG_LOG | TEMPEST_ARG_DAEMON))
#define TEMPEST_INV_TRACE(c)    (c & ~(TEMPEST_ARG_TRACE | TEMPEST_ARG_FORMAT | TEMPEST_ARG_INTERVAL | TEMPEST_ARG_LOG))
#define TEMPEST_INV_STOP(c)     (c & ~(TEMPEST_ARG_STOP))
#define TEMPEST_INV_VERSION(c)  (c & ~(TEMPEST_ARG_VERSION))
#define TEMPEST_INV_HELP(c)     (c & ~(TEMPEST_ARG_HELP | TEMPEST_ARG_EMPTY))

namespace tempest {

using namespace std;

class Arguments {
public:

  enum DataFormat {
    JSON = 0,
    REST = 1,
    ECOWITT = 2
  };

private:

  static const char* usage_[];                                  // see initialization below
  static const struct option option_[];                         // see initialization below 

  static bool FormatNum2Enum(int num, DataFormat& format) {
    const DataFormat format_native[3]{DataFormat::JSON, DataFormat::REST, DataFormat::ECOWITT};

    if (num < 1 || num > 2) return (false);
    format = format_native[num];
    return (true);
  }

  static bool LogNum2Enum(int num, LogLevel& log) {
    const LogLevel log_native[5]{LogLevel::OFF, LogLevel::ERROR, LogLevel::WARN, LogLevel::INFO, LogLevel::DEBUG};

    if (num < 1 || num > 4) return (false);
    log = log_native[num];
    return (true);
  }

  string url_;
  DataFormat format_;
  int interval_;
  LogLevel log_;

  int format_num_;
  int log_num_;

  int cmdl_;

  static string Trim(const char* str) {
    //  
    // remove leading "=" and leading and trailing spaces
    //
    if (!str) str = "";
           
    return (regex_replace(str, regex("^[=\\s\\t]+|[\\s\\t]+$"), ""));
  }

  static string ShortOptions(void) {
    //
    // build getopt_long() short options from long options data structure
    //
    string opt = "-";

    for (int idx = 0; option_[idx].name; idx++) {
      opt += option_[idx].val;
      if (option_[idx].has_arg) opt += ':';
    } 

    return (opt);
  }

public:

  static void PrintCommandLine(int argc, char* const argv[], string& str) {
    //
    // print original command line
    //
    ostringstream text{""};

    for (int idx = 0; idx < argc; idx++) {
      if (!idx) text << argv[idx];
      else text << " " << argv[idx];
    }
    str = text.str();
  }

  static void PrintUsage(string& str) {
    //
    // print usage based on an array of strings
    //
    ostringstream text{""};

    for (int idx = 0; usage_[idx]; idx++) text << usage_[idx] << endl;
    str = text.str();
  }

  Arguments(int argc, char* const argv[]) {
    //
    // Parse the command line and verify its syntax and semantics validity
    //

    // initialize options to default state
    url_ = "";
    format_ = DataFormat::REST;
    interval_ = 1;
    log_ = LogLevel::INFO;

    format_num_ = 1;
    log_num_ = 3;

    cmdl_ = 0;
  
    try {
      //
      // check command line syntax
      //
      int value, num;
      string arg, option_short;

      opterr = 0;               // silence getopt_long()
      option_short = ShortOptions();

      while ((value = getopt_long(argc, argv, option_short.c_str(), option_, nullptr)) != -1) {
        arg = Trim(optarg);

        switch (value) {
          case 'u':
            if (arg.empty()) throw invalid_argument(arg);
            url_ = arg;

            cmdl_ |= TEMPEST_ARG_URL;
            break;

          case 'f':
            num = stoi(arg);
            if (!FormatNum2Enum(num, format_)) throw out_of_range(arg);
            format_num_ = num;

            cmdl_ |= TEMPEST_ARG_FORMAT;
            break;

          case 'i':
            num = stoi(arg);
            if (num < 1 || num > 30) throw out_of_range(arg);
            interval_ = num;

            cmdl_ |= TEMPEST_ARG_INTERVAL;
            break;

          case 'l':
            num = stoi(arg);
            if (!LogNum2Enum(num, log_)) throw out_of_range(arg);
            log_num_ = num;

            cmdl_ |= TEMPEST_ARG_LOG;
            break;

          case 'd':
            cmdl_ |= TEMPEST_ARG_DAEMON;
            break;

          case 't':
            cmdl_ |= TEMPEST_ARG_TRACE; 
            break;

          case 's':
            cmdl_ |= TEMPEST_ARG_STOP;     
            break;

          case 'v':
            cmdl_ |= TEMPEST_ARG_VERSION;      
            break;

          case 'h':
            cmdl_ |= TEMPEST_ARG_HELP;  
            break;

          default:
            throw invalid_argument(arg);
        }
      }
    
      //
      // check command line semantics
      //
      if (TEMPEST_REQ_RELAY(cmdl_)) {
        // relay command
        if (TEMPEST_INV_RELAY(cmdl_)) throw invalid_argument("relay");
      }
      else if (TEMPEST_REQ_TRACE(cmdl_)) {
        // trace command
        if (TEMPEST_INV_TRACE(cmdl_)) throw invalid_argument("trace");

        if (TEMPEST_UDP_TRACE(cmdl_)) {
          format_ = DataFormat::JSON;
          interval_ = 0;
          format_num_ = 0;
        }
      }
      else if (TEMPEST_REQ_STOP(cmdl_)) {
        // stop command
        if (TEMPEST_INV_STOP(cmdl_)) throw invalid_argument("stop");
      }
      else if (TEMPEST_REQ_VERSION(cmdl_)) {
        // version command
        if (TEMPEST_INV_VERSION(cmdl_)) throw invalid_argument("version");
      }
      else if (TEMPEST_REQ_HELP(cmdl_)) {
        // help command
        if (TEMPEST_INV_HELP(cmdl_)) throw invalid_argument("help");
      }
      else {
        // empty command line
        if (cmdl_) throw invalid_argument("invalid command");
        cmdl_ |= TEMPEST_ARG_EMPTY;
      }
    }
    catch (exception const & ex) {
      // invalid command line
      cmdl_ |= TEMPEST_ARG_INVALID;
    }
  }

  bool IsCommandLineInvalid(void) const {
    //
    // Return whether the command line is invalid or not
    //
    return (cmdl_ & TEMPEST_ARG_INVALID);
  }

  bool IsCommandLineEmpty(void) const {
    //
    // Return whether the command line is empty or not
    //
    return (cmdl_ & TEMPEST_ARG_EMPTY);
  }

  bool IsCommandRelay(string& url, DataFormat& format, int& interval, LogLevel& log, bool& daemon, string& str) const {
    //
    // Return whether the relay command was invoked and all its parameters 
    //
    if (TEMPEST_INV_RELAY(cmdl_)) return (false);
    
    url = url_;
    format = format_;
    interval = interval_;
    log = log_;
    daemon = cmdl_ & TEMPEST_ARG_DAEMON;

    ostringstream text{""};

    text << "tempest --url=" << url_;
    text << " --format=" << format_num_;
    text << " --interval=" << interval_;    
    text << " --log=" << log_num_;
    if (daemon) text << " --daemon";
    str = text.str();

    return (true);
  }

  bool IsCommandTrace(DataFormat& format, int& interval, LogLevel& log, string& str) const {
    //
    // Return whether the trace command was invoked and all its parameters 
    //
    if (TEMPEST_INV_TRACE(cmdl_)) return (false);
    
    format = format_;
    interval = interval_;
    log = log_;
    
    ostringstream text{""};
    
    text << "tempest --trace";
    text << " --format=" << format_num_;
    text << " --interval=" << interval_;    
    text << " --log=" << log_num_;
    str = text.str();

    return (true);
  }

  bool IsCommandStop(string& str) const {
    //
    // Return whether the stop command was invoked 
    //
    if (TEMPEST_INV_STOP(cmdl_)) return (false);

    str = "tempest --stop";

    return (true);
  }

  bool IsCommandVersion(string& str) const {
    //
    // Return whether the version command was invoked 
    //
    if (TEMPEST_INV_VERSION(cmdl_)) return (false);

    str = "tempest --version";

    return (true);
  }

  bool IsCommandHelp(string& str) const {
    //
    // Return whether the help command was invoked 
    //
    if (TEMPEST_INV_HELP(cmdl_)) return (false);
    
    str = "tempest [--help]";

    return (true);
  }
};

// Static Initialization ------------------------------------------------------------------------------------------------------

const char* Arguments::usage_[] = {
  "Usage:        tempest [OPTIONS]",
  "",
  "Commands:",
  "",    
  "Relay:        tempest --url=<url> [--format=<fmt>] [--interval=<min>]",
  "                      [--log=<lev>] [--daemon]",
  "Trace:        tempest --trace [--format=<fmt>] [--interval=<min>]",
  "                      [--log=<lev>]",  
  "Stop:         tempest --stop",
  "Version:      tempest --version",
  "Help:         tempest [--help]",
  "",
  "Options:",
  "",
  "-u | --url=<url>      full URL to relay data to",
  "-f | --format=<fmt>   format to which the UDP data is repackaged:",
  "                      1) REST API, 2) Ecowitt (default if omitted: 1)",
  "-i | --interval=<min> interval in minutes at which data is relayed:",
  "                      1 <= min <= 30 (default if omitted: 1)",
  "-l | --log=<lev>      1) only errors",
  "                      2) errors and warnings",
  "                      3) errors, warnings and info (default if omitted)",
  "                      4) errors, warnings, info and debug (everything)",
  "-d | --daemon         run as a service",
  "-t | --trace          relay data to the terminal standard output",
  "                      (if both --format and --interval are omitted",
  "                      the source UDP JSON will be traced instead)",
  "-s | --stop           stop relaying/tracing and exit gracefully", 
  "-v | --version        print version information",
  "-h | --help           print this help",
  "",
  "Examples:",
  "",
  "tempest --url=http://hubitat.local:39501 --format=2 --interval=5",
  "tempest -u=192.168.1.100:39500 -l=2 -d",
  "tempest --stop",
  nullptr
};

const struct option Arguments::option_[] = {
  {"url",      required_argument, 0, 'u'},
  {"format",   required_argument, 0, 'f'},
  {"interval", required_argument, 0, 'i'},
  {"log",      required_argument, 0, 'l'},
  {"daemon",   no_argument,       0, 'd'},   
  {"trace",    no_argument,       0, 't'},    
  {"stop",     no_argument,       0, 's'},
  {"version",  no_argument,       0, 'v'},    
  {"help",     no_argument,       0, 'h'},
  {nullptr,    0,                 0, 0  }
};

} // namespace tempest

// Recycle Bin ----------------------------------------------------------------------------------------------------------------

/*

*/

// EOF ------------------------------------------------------------------------------------------------------------------------

#endif // TEMPEST_ARGUMENTS
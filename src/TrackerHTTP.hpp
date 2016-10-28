/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        TrackerHTTP
//- Description:  Class declaration for HTTP-based usage tracker
//-               
//- Owner:        Brian Adams, Sandia National Laboratories

#ifndef TRACKER_HTTP_H
#define TRACKER_HTTP_H

#include <string>
#include <vector>
#include <list>
#include <curl/curl.h>
#include "dakota_system_defs.hpp"

namespace Dakota {

// forward declarations
class ProblemDescDB;

/// TrackerHTTP: a usage tracking module that uses HTTP/HTTPS via the
/// curl library
class TrackerHTTP {

public:

  /// default constructor is allowed, but doesn't generate output
  TrackerHTTP();

  /// standard constructor with ProblemDescDB, rank
  TrackerHTTP(int world_rank = 0);

  /// destructor to free handles
  ~TrackerHTTP();

  /// post the start of an analysis and archive start time
  void post_start(ProblemDescDB& problem_db);

  /// post the completion of an analysis including elapsed time
  void post_finish(unsigned runtime = 0); 


private:

  /// shared initialization functions across constructors
  void initialize(int world_rank = 0);

  /// append keyword/value pair to url in GET style (with &keyword=value);
  /// set delimit = false to omit the &
  void url_add_field(std::string& url, 
		     const char* keyword, 
		     const std::string& value,
		     bool delimit = true) const;

  /// construct URL with shared information for start/finish
  void build_default_data(std::string& url, std::time_t& rawtime,
			  const std::string& mode) const;

  /// transmit data to the web server using GET
  void send_data_using_get(const std::string& urltopost) const;

  /// POST separate location and query; datatopost="name=daniel&project=curl"
  void send_data_using_post(const std::string& datatopost);

  /// Split a string on a delimiter and place tokens in elems
  void split_string(const std::string &s, const char &delim, 
		std::vector<std::string> &elems);

  /// Populate serverList with tracker and proxy URLs from dt
  void parse_tracking_string(const std::string &dt);

  // ----------------------------
  // helpers to get tracking data
  // ----------------------------

  /// extract list of methods from problem database 
  void populate_method_list(ProblemDescDB& problem_db);

  /// get the real user ID
  std::string get_uid() const;

  /// get the username as reported by the environment
  std::string get_username() const;

  /// get the system hostname
  std::string get_hostname() const;

  /// get the operating system
  std::string get_os() const;

  /// get the date and time as a string YYYYMMDDHHMMSS
  std::string get_datetime(const std::time_t& rawtime) const;


  // ------------
  // private data
  // ------------

  /// pointer to the curl handler instance
  CURL *curlPtr;

  /// pointer to /dev/null
  FILE *devNull;

  /// struct to hold tracker/proxy pairs
  struct Server {
    Server(std::string t, std::string p) : tracker(t), proxy(p) {};
    std::string tracker;
    std::string proxy;
  };

  /// List of servers to try (tracker and proxy)
  std::list<Server> serverList;

  /// seconds until the request will timeout (may have issues with signals)
  long timeoutSeconds;

  /// list of active methods
  std::string methodList;

  /// DAKOTA version
  std::string dakotaVersion;

  /// cached starting time in raw seconds
  std::time_t startTime;

  /// verbosity control
  short outputLevel;

  // TODO: possible additions
  //std::string startTimeString;
  //std::string userID;
  //std::string userName;
  //std::string hostName;

};

} // namespace Dakota

#endif

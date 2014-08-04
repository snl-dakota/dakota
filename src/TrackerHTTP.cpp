/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        TrackerHTTP
//- Description:  Implementation of HTTP-based usage tracker
//-               
//- Owner:        Brian Adams, Sandia National Laboratories

#include "dakota_system_defs.hpp"
#include <unistd.h>
// for uname
#include <sys/utsname.h>
#include "dakota_data_types.hpp"
#include "TrackerHTTP.hpp"
#include "DakotaIterator.hpp"
#include "DakotaBuildInfo.hpp"

/*
  TODO

  * Track:
    - info on parallelism mode?
    - architecture type?
    - evaluation count? 
    - analysis_driver? (likely not)

  * Web server could return a unique ID to the tracker for use in
  posting the finish.

  * Setup all static data for transmission, except timing, which will
  be done in start/end?

  * Use DAKOTA timing data instead of coarse seconds

  * Handle case of invalid URL or proxy coming from configure (don't
  initialize curl)

  * Optional debug info
*/

namespace Dakota {

enum {TH_SILENT_OUTPUT, TH_QUIET_OUTPUT, TH_NORMAL_OUTPUT, TH_VERBOSE_OUTPUT, 
      TH_DEBUG_OUTPUT};

//
// Public implementations
//

//TrackerHTTP::TrackerHTTP(): 
//  curlPtr(NULL), devNull(NULL), timeoutSeconds(2), dakotaVersion("unknown"), 
//  outputLevel(TH_SILENT_OUTPUT)
//{ initialize(); }

//TrackerHTTP::TrackerHTTP(ProblemDescDB& problem_db, int world_rank): 
TrackerHTTP::TrackerHTTP(int world_rank): 
  curlPtr(NULL), devNull(NULL), timeoutSeconds(2), dakotaVersion("unknown"),
  outputLevel(TH_SILENT_OUTPUT)
{
  initialize(world_rank);
  //if (curlPtr != NULL)
  //  populate_method_list(problem_db); // MSE: postpone
}

TrackerHTTP::~TrackerHTTP() 
{
  if (devNull)
    std::fclose(devNull);

  if (curlPtr) {
    curl_easy_cleanup(curlPtr);
    curlPtr = NULL;
 } 
}

//void TrackerHTTP::post_start()
void TrackerHTTP::post_start(ProblemDescDB& problem_db)
{
  if (curlPtr == NULL) return;
  else                 populate_method_list(problem_db); // MSE: postponed

  startTime = std::time(NULL);  // archive the start time
  
  // POST default
  std::string datatopost;
  build_default_data(datatopost, startTime, "START");
  //send_data_using_post(datatopost.str());
  send_data_using_post(datatopost);

  // GET example (deprecated)
  /*
  std::stringstream urltoget;
  urltoget << trackerLocation; // assumes no "?"
  urltoget << "?";             // TODO: detect whether ? is needed
  build_default_data(urltoget, startTime, "START");
  send_data_using_get(urltoget.str());
  */
}

void TrackerHTTP::post_finish(unsigned runtime) 
{
  if (curlPtr == NULL)
    return;

  std::time_t end_time = std::time(NULL);

  std::string datatopost;
  build_default_data(datatopost, end_time, "FINISH");
  // add the elapsed time in the postrun case
  std::string elapsed_time( boost::lexical_cast<std::string>(end_time -
                                                             startTime) );
  url_add_field(datatopost, "elapsedseconds", elapsed_time);

  send_data_using_post(datatopost);
}


// 
// private implementations
//

// Initialization helpers

void TrackerHTTP::initialize(int world_rank)
{
  // Avoid tracking if user specified none or on rank > 0
  char *ptr_notrack = std::getenv("DAKOTA_NO_TRACKING");
  if (ptr_notrack != NULL || world_rank > 0)
    return;

  dakotaVersion = DakotaBuildInfo::get_rev_number();

  curlPtr = curl_easy_init();
  if (!curlPtr && outputLevel > TH_QUIET_OUTPUT)
    Cerr << "\nWarning (TrackerHTTP): Could not initialize curl." << std::endl;
  else if (outputLevel > TH_NORMAL_OUTPUT)
    Cout << "TrackerHTTP: Curl initialized" << std::endl;

  if (curlPtr == NULL)
    return;

  // get the URL and optional proxy from configure
#ifdef DAKOTA_USAGE_TRACKING
  std::string dt = DAKOTA_USAGE_TRACKING;
#else
  std::string dt;
#endif
  size_t delim_loc = dt.find(';');
  if (delim_loc == std::string::npos)
    trackerLocation = dt;
  else {
    trackerLocation = dt.substr(0, delim_loc);
    proxyLocation = dt.substr(delim_loc+1);
  }

  // override default behavior where empty string means DISALLOW proxy
  if (proxyLocation.size() > 0)
    curl_easy_setopt(curlPtr, CURLOPT_PROXY, proxyLocation.c_str());

  curl_easy_setopt(curlPtr, CURLOPT_TIMEOUT, timeoutSeconds);

  // TODO: get return info from the request
  devNull = std::fopen("/dev/null", "w");
  curl_easy_setopt(curlPtr, CURLOPT_WRITEDATA, devNull);

  // for testing, don't verify SSL cert
  curl_easy_setopt(curlPtr, CURLOPT_SSL_VERIFYPEER, 0);

}

void TrackerHTTP::populate_method_list(ProblemDescDB& problem_db)
{
  // TODO: consider limiting if too many methods
  // TODO: consider sorting and removing duplicates -- probably not
  // TODO: fix trailing comma
  // TODO: consider "uses_method"
  const IteratorList& dakota_iterators = problem_db.iterator_list();
  IteratorList::const_iterator itit  = dakota_iterators.begin();
  IteratorList::const_iterator itend = dakota_iterators.end();
  for ( ; itit != itend; ++itit)
    methodList += itit->method_string() + ",";
}


// URL creation helpers

void TrackerHTTP::build_default_data(std::string& data, std::time_t& rawtime, 
				     const std::string& mode) const
{
  url_add_field(data, "mode", mode, false); // first field gets no &
  url_add_field(data, "gmt", get_datetime(rawtime));
  url_add_field(data, "uid", get_uid());
  url_add_field(data, "username", get_username());
  url_add_field(data, "hostname", get_hostname());
  url_add_field(data, "os", get_os());
  url_add_field(data, "methods", methodList);
  url_add_field(data, "version", dakotaVersion);
}

void TrackerHTTP::url_add_field(std::string& url, 
			       const char* keyword, 
			       const std::string& value,
			       bool delimit) const
{
  url += delimit ? std::string("&") + keyword + "=" + value
                 : std::string(keyword) + "=" + value;
}


// transmit helpers

/// whole url including location&fields
void TrackerHTTP::send_data_using_get(const std::string& urltopost) const
{
  if (outputLevel > TH_NORMAL_OUTPUT)
    Cout << "GETting URL:\n" << urltopost << std::endl;

  char* cstr_url = new char[urltopost.size()+1];
  std::strcpy(cstr_url, urltopost.c_str());
  curl_easy_setopt(curlPtr, CURLOPT_URL, cstr_url);
  CURLcode res = curl_easy_perform(curlPtr);
  if (res != 0 && outputLevel > TH_QUIET_OUTPUT)
    Cerr << "curl_easy_perform returned " << res << std::endl;
  
  delete[] cstr_url;
}

/// separate location and query; datatopost="name=daniel&project=curl"
void TrackerHTTP::send_data_using_post(const std::string& datatopost) const
{
  if (outputLevel > TH_NORMAL_OUTPUT)
    Cout << "POSTing data:\n" << datatopost << "\nto URL\n" << trackerLocation 
	 << std::endl;


  char* cstr_location = new char[trackerLocation.size()+1];
  std::strcpy(cstr_location, trackerLocation.c_str());
  curl_easy_setopt(curlPtr, CURLOPT_URL, cstr_location);

  char* cstr_data = new char[datatopost.size()+1];
  std::strcpy(cstr_data, datatopost.c_str());
  curl_easy_setopt(curlPtr, CURLOPT_POSTFIELDS, cstr_data);   

  CURLcode res = curl_easy_perform(curlPtr);
  if (res != 0 && outputLevel > TH_QUIET_OUTPUT)
    Cerr << "curl_easy_perform returned " << res << std::endl;
  
  delete[] cstr_data;
  delete[] cstr_location;
}


// Helper functions to retreive instance-specific data

std::string TrackerHTTP::get_uid() const
{
  uid_t myuid = getuid();
  std::ostringstream uid_str;
  uid_str << myuid;
  return uid_str.str();
}

std::string TrackerHTTP::get_username() const
{
  char *whoami = std::getenv("USER");
  return std::string(whoami);
}

std::string TrackerHTTP::get_hostname() const
{
  // WARN: HOST_NAME_MAX may need explicit inclusion/definition
  // not available on all platforms; use 255 instead
  std::string hostname_str;
  char myhost[255];
  if (gethostname(myhost, 255) == 0)
    hostname_str = myhost;
  else
    hostname_str = "unknown";
  return hostname_str;
}

std::string TrackerHTTP::get_os() const
{
  std::string os_str;
  struct utsname myuname;
  if (uname(&myuname) >= 0)
    os_str = myuname.sysname;
  else
    os_str = "unknown";
  return os_str;
}

std::string TrackerHTTP::get_datetime(const std::time_t& mytime) const
{
  // using GMT for cross-site tracking
  struct std::tm* time_tm = std::gmtime(&mytime);
  const size_t MAX_DATE_LEN = 15; // YYYYMMDDHHMMSS + '\0'
  char timestr[MAX_DATE_LEN];
  std::strftime(timestr, MAX_DATE_LEN, "%Y%m%d%H%M%S", time_tm);
  return std::string(timestr);
}

} // namespace Dakota

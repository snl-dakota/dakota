#pragma once
#ifndef DAKOTA_ROL_OUTPUT_STREAM
#define DAKOTA_ROL_OUTPUT_STREAM

#include <boost/iostreams/filter/line.hpp>
#include <boost/iostreams/filtering_stream.hpp>

namespace rol_interface {

// -----------------------------------------------------------------------------
/** OutputStreamFilter wraps a pointer to a std::ostream and prepends "ROL: " to the 
    beginning of each new line 

    NOTE: This is known to be compatible with Boost 1.69, but not 1.81
*/

class OutputStreamFilter {
public:
  OutputStreamFilter( std::ostream* os_ptr ) {
    filterStream.push(LineFilter());
    filterStream.push(os_ptr);
  }

  ~OutputStreamFilter() {
    filterStream.flush();
  }

private:

  class LineFilter : public boost::iostreams::line_filter  {
    std::string do_filter( const std::string& line ) {
      return "ROL: " + line;
    }
  };

  boost::iostreams::filtering_ostream filterStream;
}; 

} // namespace rol_interface

#endif // DAKOTA_ROL_OUTPUT_STREAM

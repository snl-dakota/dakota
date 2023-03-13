#pragma once
#ifndef DAKOTA_ROL_OUTPUT_STREAM
#define DAKOTA_ROL_OUTPUT_STREAM

namespace rol_interface {

// -----------------------------------------------------------------------------
/** OutputStreamFilter wraps a pointer to a std::ostream and prepends "ROL: " to the 
    beginning of each new line */

class OutputStreamFilter {
public:

  OutputStreamFilter( std::ostream* os_ptr=Dakota::Cout ) {
    filterStream.push(LineFilter());
    filterStream.push(os_ptr);
  }

  ~OutputStream() {
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

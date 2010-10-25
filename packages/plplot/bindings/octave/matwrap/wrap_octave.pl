#
# Language module for the wrapper generator for octave 2.0.  Most of these
# functions are called directly from the wrapper generator.
#
# Octave is a little difficult to handle because (like MATLAB) it can only
# define a single C function per mex file.  This means that the C function must
# take an additional argument of which sub-function to call.  We provide octave
# functions which supply the extra argument.
#
# Copyright (c) 1997 Gary R. Holt.  This is distributed under the terms of the 
# perl artistic license (http://language.perl.com/misc/Artistic.html).
#

package octave;			# Everything should be in this package.

@function_names = ();		# This table translates function names and
				# their assigned IDs.  The extra numerical
				# argument to the mexFunction() indicates
				# which function the user actually wanted to
				# call.

$max_dimensions = 2;		# Maximum number of dimensions in tensors.

*pointer_type_code = *main::pointer_type_code;
				# Copy the definition of pointer_type_code
				# into this module.

#
# arg_pass(\%function_def, $argname)
#
# A C or C++ expression used to pass the argument to another function
# which does not know anything about the type of the argument.  For
# example, in the MATLAB module this function returns an expression for
# the mxArray type for a given argument.
#
sub arg_pass {
  my ($faa, $argname) = @_;	# Name the arguments.
  $faa->{args}{$argname}{oct_expr}; # Return the octave expression.
}

#
# arg_declare("arg_name_in_arglist")
# 
# This returns a C/C++ declaration appropriate for the argument passed
# using arg_pass.  For example, in the MATLAB module this function returns
# "mxArray *arg_name_in_arglist".
#
sub arg_declare {
  "const octave_value &$_[0]";
}


#
# declare_const("constant name", "class name", "<type>", "doc str")
#
# Output routines to make a given constant value accessible from the 
# interpreter.
# If "class name" is blank, this is a global constant.
#
sub declare_const {
				# This is not currently supported.
}


#
# error_dimension(\%function_def, $argname)
#
# A C statement (including the final semicolon, if not surrounded by braces)
# which indicates that an error has occured because the dimension of argument
# $argname was wrong.
#
sub error_dimension {
  "{ error(\"dimension mismatch in argument $_[1]\"); return retval; }"
				# Return whatever we've made so far, which
				# is probably an empty list.
}

#
# finish()
#
# Called after all functions have been wrapped, to close the output file and do
# whatever other cleanup is necessary.
#
sub finish {
  print OUTFILE qq[

//
// The main dispatch function.  This function calls the appropriate wrapper
// based on the value of the first argument.
//
DEFUN_DLD($octave_output_file, args, nargout, "See $stub_file for documentation")
{
  int function_id = int(args(0).double_value());
				// Get the function ID.
  switch (function_id)
  {
];
				# Output the beginning of the definition.
  foreach (0 .. @function_names-1) {
    print "  case $_: return $function_names[$_](args, nargout);\n";
  }
				# Dispatch appropriately.
  print "
  default: error(\"Illegal function ID parameter\"); return octave_value();
  }
}

";

  close(OUTFILE);		# Done with this file.
  close(STUBFILE);		# We're also done with the stub file.
}


#
# Begin the definition of a function.  Arguments:
# 1) The %function_def array.
#
sub function_start {
  my $faa = $_[0];		# Access the argument.

  my $retstr;

  my $fname = $faa->{script_name} ||
    ($faa->{class} ? $faa->{class} . "_" : "") . $faa->{name};
				# Get octave's name for the function.
  $faa->{octave_name} = $fname;	# Remember that for later.

#
# Octave doesn't support modify variables, so we add an extra variable to
# the output argument list for each input modify variable.
#
  $retstr = sprintf("octave_value_list _wrap_%s(const octave_value_list &args, int %s)\n",
		    $fname,
		    (@{$faa->{outputs}} + @{$faa->{modifies}}) <= 1 ? '' : 'nargout');
				# Don't specify nargout unless we actually
				# need to test it.

  $retstr .= ("{\n" .
	      "  octave_value_list retval(" . @{$faa->{outputs}} . ", octave_value());\n" . # Where we return stuff.
				# Make sure we have right # of args:
	      "  if (args.length()-1 != " . (scalar(@{$faa->{inputs}}) + 
					     scalar(@{$faa->{modifies}})));
  $retstr .= " ||\n      nargout != " . (scalar(@{$faa->{outputs}}) + scalar(@{$faa->{modifies}}))
    unless (@{$faa->{outputs}}+@{$faa->{modifies}}) <= 1;
				# Permissible not to specify output args
				# if there is only one.
  $retstr .= (")\n" .
	      "  {\n" .
	      "    print_usage(\"$fname\");\n" . # Give an error.
	      "    return retval;\n" . # Quit now.
	      "  }\n\n");
#
# Now go through and store the octave expression to get each argument, so
# we can access them later:
#
  my $idx = 1;			# Start input arguments at 1, since the
				# first argument is the function ID.
  my $arg;
  foreach (@{$faa->{inputs}}, @{$faa->{modifies}}) {
    $arg = $faa->{args}{$_};	# Point to this argument.
    $arg->{oct_expr} = "args(" . $idx++ . ")";

    if ($arg->{basic_type} eq 'char *') {
      if (@{$arg->{dimension}} != 0) { # A vector of characters?
	die("wrap_octave: vectors of strings are not currently supported\n");
      }
      $arg->{vectorize} = 0;	# We don't support vectorization of strings
				# since octave only has the klugy char
				# matrix for string vectors.
      $arg->{source} eq 'modify' and $faa->{vectorize} = 0;
				# If a modify argument can't be vectorized,
				# we can't vectorize the function.
    }
  }

  $idx = 0;			# Now do the same for the outputs.
  foreach (@{$faa->{outputs}}) { # These have different indices.
    $arg = $faa->{args}{$_};	# Point to this argument.
    $arg->{oct_expr_out} = "retval(" . $idx++ . ")";
    if ($arg->{basic_type} eq 'char *') {
      if (@{$arg->{dimension}} != 0) { # A vector of characters?
	die("wrap_octave: vectors of strings are not currently supported\n");
      }
      $arg->{vectorize} = 0;	# We don't support vectorization of strings
				# since octave only has the klugy char
				# matrix for string vectors.
      $faa->{vectorize} = 0;	# This function can't be vectorized if its
				# output arguments can't be vectorized.
    }
  }

  foreach (@{$faa->{modifies}}) { # Indicate where we're supposed to put the
				# modified output as well.
    $faa->{args}{$_}{oct_expr_out} = "retval(" . $idx++ . ")";
  }

  $retstr;
}

#
# function_end(\%function_def)
#
# Return a string which finishes off the definition of a wrapper.
#
sub function_end {
  my ($faa) = @_;		# Access the argument.

  my $fname = $faa->{octave_name}; # Get octave's name for the function.
  push(@function_names, "_wrap_$fname"); # Register it in our list of functions.

#
# Add a function to the stub file:
#
  my $outargs = join(", ", @{$faa->{outputs}}, # List output arguments.
		     map { "out_$_" } @{$faa->{modifies}});
				# Modify arguments also must appear in the
				# output list.
  (@{$faa->{outputs}}+@{$faa->{modifies}}) > 1 and $outargs = "[$outargs]";
				# If there's more than one, put it in
				# brackets.
  $outargs ne '' and $outargs .= " = ";	# Add an equals sign too.
  my $inargs = join(", ", @{$faa->{inputs}}, @{$faa->{modifies}});
				# Format the input arguments.
  print(STUBFILE
	"function $outargs$fname($inargs)\n",
	"% $outargs$fname($inargs)\n",
	"% No documentation is available yet.\n",
	"  $outargs$octave_output_file(",
	join(", ", scalar(@function_names)-1, @{$faa->{inputs}}, @{$faa->{modifies}}),
	");\n",
	"endfunction\n\n");


  "  return retval;\n}\n\n";		# Just close off the opening brace.
}

#
# get_c_arg_scalar(\%function_def, $argname)
#
# Returns C statements to load the current value of the given argument
# into the C variable C<$function_def{args}{$argname}{c_var_name}>.  The
# variable is guaranteed to be either a scalar or an array with dimensions
# 1,1,1....
#
sub get_c_arg_scalar {
  my ($faa, $argname) = @_;	# Name the arguments.
  my $arg = $faa->{args}{$argname}; # Access the definition of this argument.

  my $argtype = $arg->{basic_type}; # Get the basic type, without any frills.

  if (exists($typemap_get_scalar{$argtype})) { # Do we understand this type?
    return &{$typemap_get_scalar{$argtype}}($arg, $argname, $argtype); # Do the conversion.
  } elsif ($argtype =~ /\*$/) { # Is this a pointer class we don't understand?
    return ("  if (!_get_pointer((void **)&$arg->{c_var_name},\n" .
	    "                    $arg->{oct_expr}.complex_matrix_value(),\n" .
	    "                    @{[pointer_type_code($argtype)]})) /* $argtype */\n" .
	    "  {\n" .
	    "    error(\"Expecting type '$argtype' for argument named $argname\");\n" .
	    "    return retval;\n" .
	    "  }\n");
				# Treat it as an array of pointers.
  } else {			# Unrecognized type?
    die("wrap_octave: don't understand type '$argtype' as scalar\n");
  }
}

#
# get_c_arg_ptr(\%function_def, $argname)
#
# Returns C statements to set up a pointer which points to the first
# value in the given argument.  The dimensions are guaranteed to be
# correct.  The type of the argument should be checked.  The pointer
# value should be stored in the variable
# $function_def{args}{$argname}{c_var_name}.
#
# The pointer should actually point to the array of all the values of
# the variable.  The array should have the same number of elements as
# the argument, since to vectorize the function, the wrapper function
# will simply step through this array.  If we want a float type and the
# input vector is double or int, then a temporary array must be made
# which is a copy of the double/int arrays.
#
sub get_c_arg_ptr {
  my ($faa, $argname) = @_; # Name the arguments.

  my $arg = $faa->{args}{$argname}; # Access the definition of this argument.

  my $argtype = $arg->{basic_type}; # Get the basic type, without any frills.

  if (exists($typemap_get_ptr{$argtype})) { # Do we understand this type?
    return &{$typemap_get_ptr{$argtype}}($arg, $argname, $argtype); # Do the conversion.
  } elsif ($argtype =~ /\*$/) { # Is this a pointer class we don't understand?
    return ("  $arg->{c_var_name} =\n" .
	    "    ($argtype *)alloca(sizeof ($argtype)*_arraylen($arg->{oct_expr}));\n" .
	    "  if (!_get_pointer((void **)$arg->{c_var_name},\n" .
	    "                    $arg->{oct_expr}.complex_matrix_value(),\n" .
	    "                    @{[pointer_type_code($argtype)]})) /* $argtype */\n" .
	    "  {\n" .
	    "    error(\"$faa->{octave_name}: expecting type $argtype for argument $argname\");\n" .
	    "    return retval;\n" .
	    "  }\n");
  } else {			# Unrecognized type?
    die("wrap_octave: don't understand type '$argtype' as scalar/matrix\n");
  }
}

#
# Get the name of the output file given the input files.  Arguments:
# 1) A reference to a list of input files.
#
sub get_outfile {
  die("wrap_octave: must explicitly specify the output file name with -o fname.c\n");
}

#
# get_size(\%function_def, $argname, $n)
#
# Returns a C expression which is the size of the n'th dimension of the given
# argument.  Dimension 0 is the least-significant dimension.
#
sub get_size {
  my ($faa, $argname, $index) = @_; # Name the arguments.
  my $arg = $faa->{args}{$argname}; # Access definition of this argument.

  "_dim($arg->{oct_expr}, $index)";
				# Always call the general purpose _dim
				# subroutine, because 
				# $arg->{oct_expr}.rows() can return 0 or
				# -1 if it is a scalar.
}

#
# Initialize the output file.  Arguments:
# 1) The name of the output file.
# 2) A reference to a list of input files explicitly listed.
# 3) A reference to the words passed to the C preprocessor.
# 4) A string that represents our guess as to the #includes which are
#    required.
#
sub initialize {
  my ($outfile, $infiles, $cpp_cmd, $include_str) = @_;	# Name the arguments.

  $stub_file or
    die("wrap_octave: must specify the name of the stub file on the command line\n");
  open(STUBFILE, "> $stub_file") ||
    die("wrap_octave: cannot create stub file \"$stub_file\"--$!\n");

  print STUBFILE "# Stub subroutines for octave file $outfile.
# This file was automatically generated for octave by wrap_octave on
# ", scalar(localtime(time)), " from
# @{[@$infiles, @$cpp_cmd]}.
#

1;				# This is not a function file.
";

  open(OUTFILE, "> $outfile") ||
    die("wrap_octave: can't open output file $outfile--$!\n");

  $octave_output_file = $outfile; # Remember the name of the output file.
  $octave_output_file =~ s/\..*$//; # Strip off the extension.
  $octave_output_file =~ s@.*/@@; # Strip off the directories.

  print OUTFILE "/*
 * This file was automatically generated for octave by wrap_octave on
 * ", scalar(localtime(time)), "
 * from @{[@$infiles, @$cpp_cmd]}.
 */

$include_str
";				# Output a header.

  while (defined($_ = <DATA>)) { # Read the rest of the static functions.
    print OUTFILE $_;
  }
				
  return "octave::OUTFILE";	# Return the file handle.
}

#
# make_output_scalar(\%function_def, $argname)
#
#   Return C code to create the given output variable, which is guaranteed
#   to be a scalar.
#
sub make_output_scalar {
  my ($faa, $argname) = @_;
  my $arg = $faa->{args}{$argname}; # Access the definition of this argument.

  my $argtype = $arg->{basic_type}; # Get the basic type, without any frills.

  if (exists($typemap_output_scalar_make{$argtype})) { # Do we understand this type?
    return &{$typemap_output_scalar_make{$argtype}}($arg);
				# Do the conversion.
  } elsif ($argtype =~ /\*$/) { # Is this a pointer class we don't understand?
    return "  $arg->{oct_expr_out} = octave_value(Complex(0, 0));\n";
				# Make a value.
  } else {			# Unrecognized type?
    die("wrap_octave: don't understand scalar output/modify type '$argtype'\n");
  }
}

#
#  make_output_ptr(\%function_def, $argname, $n_dimensions, @dimensions)
#
# Return C code to set up the given output variable.  $n_dimensions is a C
# expression, not a constant.  @dimensions is a list of C expressions that are
# the sizes of each dimension.  There may be more values in @dimensions than 
# are wanted.
#
sub make_output_ptr {
  my ($faa, $argname, $n_dims, @dims) = @_;

  my $arg = $faa->{args}{$argname}; # Access the argument info.
  my $type = $arg->{basic_type}; # Access the type.

  my $retstr = '';

  push(@dims, 1) while (@dims < 2); # Make the first two dimensions 1 if it's
				# a scalar, since that's the way octave
				# represents it.

#
# Make an array containing the appropriate dimensions:
#

#
# Now make an argument of the appropriate type.  Note that although octave
# understands RowVector/ColumnVector as different object types from a
# Matrix, when the value gets put into an octave variable these are all 
# converted to an octave_matrix, which is just a wrapper around the Matrix
# class.  So we just use the Matrix class and don't try to figure out whether
# it's a row or column vector.
#
#
  if (exists($typemap_output_array_make{$type})) { # Do we understand this type?
    $retstr .= &{$typemap_output_array_make{$type}}($arg, $argname, @dims);
  } elsif ($type =~ /\*$/) {	# Some random pointer type?
    $retstr .= ("    $arg->{oct_expr_out} = ComplexMatrix($dims[0],$dims[1]);\n" .
				# Allocate an array to store the result.
		"    $arg->{c_var_name} = ($type *)alloca(_arraylen($arg->{oct_expr_out}) * sizeof ($type));\n");
				# Allocate a temporary array to put ptrs into.
  } else {
    die("wrap_octave: can't handle output of type '$type'\n");
  }

  $retstr;
}

#
# n_dimensions(\%function_def, $argname)
#
# Returns a C expression which is the number of dimensions of the argument 
# whose name is $argname.
#
sub n_dimensions {
  my ($faa, $argname) = @_; # Name the arguments.

  my $arg = $faa->{args}{$argname}; # Point to argument description.

  return ($arg->{basic_type} eq 'char *') ? '0' :
    "_n_dims($arg->{oct_expr})";
}


#
# Parse the argument vector for language-specific options.  Arguments:
# 1) A reference to the argument vector.
# Any arguments we use should be removed from the argument vector.
#
sub parse_argv {
  my ($ARGV) = @_;		# Access the arguments.
  my $argidx;

  for ($argidx = 0; $argidx < @$ARGV; ++$argidx) { # Look at the arguments:
    if ($ARGV->[$argidx] eq '-stub') { # Stub file name specified?
      $stub_file = $ARGV[$argidx+1]; # Get the file name.
      splice(@$ARGV, $argidx, 2); # Remove these two elements from the array.
      $argidx--;		# Back up to account for the argument we removed.
    }
  }
}

#
# Returns code to convert to and from pointer types to the languages
# internal representation, if any special code is needed.  If this
# subroutine is not called, then there are no class types and pointers
# will not need to be handled.
#
sub pointer_conversion_functions {
  ($_ = <<"###END") =~ s/^# ?//mg; # Strip the '# ' from beginning of lines.
# //
# // Pointers are stored as double precision numbers (actually, as the real part
# // of a complex number, with the imaginary part being the type).  This
# // should work even on 64 bit machines, since a double is at least 64 bits
# // on all machines that I know of.
# //
# union d_to_p {
#   double dval;			// Double precision value.
#   void *pval;			// Pointer value.
# };

# //
# // Local functions dealing with pointers:
# //
# // Pointers are stored as complex numbers.  The real part is the pointer value,
# // and the complex part is the type code.  Since the real part will be a double
# // precision number, the pointer is guaranteed to fit, even on 64-bit machines.
# //
# // Fill an array with pointers.  Arguments:
# // 1) A pointer to the array to fill.  The array is already the correct size.
# // 2) A complex matrix which contains the pointers.
# // 3) The pointer type code.
# //
# // Returns false if some of the pointers have the wrong type.
# //
# static int
# _get_pointer(void **arr, const ComplexMatrix &c_mat, unsigned typecode)
# {
#   for (int i = 0; i < c_mat.dim1(); ++i)
#     for (int j = 0; j < c_mat.dim2(); ++j)
#     {
#       d_to_p c_real;
#       Complex cval = c_mat(i,j); // Get the number.
#       unsigned this_typecode = unsigned(cval.imag()); // Get the type.
#       c_real.dval = cval.real(); // Get the address.
#       if (this_typecode != typecode) // Type does not match?
#       {
# 	*arr = __cvt_type(c_real.pval, this_typecode, typecode);
# 				// Check for inheritance.
# 	if (*arr++ == 0)
# 	  return false;		// Urk!  A type error.
#       }
#       else			// Type matches exactly:
# 	*arr++ = c_real.pval;
#     }

#   return true;			// No type error.
# }

# //
# // Load an octave object with a pointer or an array of pointers:
# //
# static void
# _put_pointer(octave_value &o_obj, void **ptr_array, double type_code) {
#   if (_n_dims(o_obj) == 0)	// Is this a scalar?
#   {
#     d_to_p c_real;
#     c_real.dval = 0;		// Zero out the unused part of the field.
#     c_real.pval = *ptr_array;	// Get the value.
#     o_obj = octave_value(Complex(c_real.dval, type_code)); // Set the object.
#   }
#   else				// It's a vector output.  In this case, the
#   {				// object's dimensions/type are already set.
#     ComplexMatrix c_mat = o_obj.complex_matrix_value();
#     Complex *c_arr = (Complex *)c_mat.data();
# 				// Point to the data.
#     for (int idx = 0; idx < c_mat.length(); ++idx)
#     {
#       d_to_p c_real;
#       c_real.dval = 0;		// Zero out the unused part of the field.
#       c_real.pval = *ptr_array++; // Get the value.
#       *c_arr++ = Complex(c_real.dval, type_code);
#     }
#     o_obj = octave_value(c_mat); // Return the value.
#   }
# }
###END
  $_;
}


#
# put_val_scalar(\%function_def, $argname)
#
# Returns C code to take the value from the C variable whose name is given
# by $function_def{args}{$argname}{c_var_name} and store it back in the
# scripting language scalar variable.
#
sub put_val_scalar {
  my ($faa, $argname) = @_;
  my $arg = $faa->{args}{$argname}; # Access the definition of this argument.

  my $argtype = $arg->{basic_type}; # Get the basic type, without any frills.

  if (exists($typemap_put_scalar{$argtype})) { # Do we understand this type?
    return &{$typemap_put_scalar{$argtype}}($arg);
				# Do the conversion.
  } elsif ($argtype =~ /\*$/) { # Is this a pointer class we don't understand?
    return "    _put_pointer($arg->{oct_expr_out}, (void **)&$arg->{c_var_name}, @{[pointer_type_code($argtype)]}); /* $argtype */\n";
  } else {			# Unrecognized type?
    die("wrap_octave: don't understand scalar output/modify type '$argtype'\n");
  }
}

#
# put_val_ptr(\%function_def, $argname)
#
# Returns C code to take the value from the C array whose name is given by
# $function_def{args}{$argname}{c_var_name} and store it back in the scripting
# language array at the specified index.  The pointer
# $function_def{args}{$argname}{c_var_name} was set up by either get_c_arg or
# make_output, depending on whether this is an input/modify or an output
# variable.
#
sub put_val_ptr {
  my ($faa, $argname) = @_;

  my $arg = $faa->{args}{$argname}; # Access the definition of this argument.

  my $argtype = $arg->{basic_type}; # Get the basic type, without any frills.
  my $argdim = $arg->{dimension}; # Point to the dimensions.

  my $retstr;

  if (exists($typemap_put_ptr{$argtype})) { # Do we understand this type?
    return &{$typemap_put_ptr{$argtype}}($arg, $argname); # Do the conversion.
  } elsif ($argtype =~ /\*$/) { # Is this a pointer class we don't understand?
    return "    _put_pointer($arg->{oct_expr_out}, (void **)$arg->{c_var_name}, @{[pointer_type_code($argtype)]}); /* $argtype */\n";
  } else {			# Unrecognized type?
    die("wrap_octave: don't understand scalar/vector output/modify type '$argtype'\n");
  }
}

###############################################################################
#
# Typemap stuff.  These are subroutines that do various things depending on the
# type.  You can add freely to this to define new types.
#

#
# Get the value of an argument which is guaranteed to be a scalar, and put
# it into a C variable.
#
# All functions in this hash are called with three arguments:
# 1) The $function_def{args}{$argname} associative array.
#    The C variable the value is stored in is $args->{c_var_name}.
# 2) The name of the argument.
# 3) The type of the argument.
#
$typemap_get_scalar{'double'} = $typemap_get_scalar{'float'} = 
  $typemap_get_scalar{'int'} = $typemap_get_scalar{'unsigned'} = 
  $typemap_get_scalar{'short'} = $typemap_get_scalar{'unsigned short'} = sub {
    my ($arg) = @_;		# Name the arguments.
    "  $arg->{c_var_name} = ($arg->{basic_type})$arg->{oct_expr}.double_value();\n";
};

$typemap_get_scalar{'char *'} = sub {
  my ($arg, $argname) = @_;	# Name the arguments.
  ("  string _$argname = $arg->{oct_expr}.string_value();\n" .
				# Get the string.
   "  _$argname += '\\0';\n" .	# Make sure it's null terminated.
   "  $arg->{c_var_name} = (char *)_$argname.data();\n");
};

$typemap_get_scalar{'char'} = sub {
  my ($arg, $argname) = @_;	# Name the arguments.
  "  $arg->{c_var_name} = *$arg->{oct_expr}.char_matrix_value().data();\n";
};

$typemap_get_scalar{'complex < double >'} = sub {
  my ($arg, $argname) = @_;	# Name the arguments.
  "  $arg->{c_var_name} = $arg->{oct_expr}.complex_value();\n";
  
};

$typemap_get_scalar{'complex < float >'} = sub {
  my ($arg, $argname) = @_;	# Name the arguments.
  ("  complex<double> _$arg->{c_var_name} = $arg->{oct_expr}.complex_value();\n" .
   "  $arg->{c_var_name} = complex<float>(_$arg->{c_var_name}.real(), $arg->{c_var_name}.imag());\n");
};

#
# Get the value of a potentially vector argument and put it into a C variable.
# The dimensions are guaranteed to be proper.  Type checking should be done.
# All functions in this hash are called with three arguments:
# 1) The $function_def{args}{$argname} associative array.
#    The C variable the value is stored in is $args->{c_var_name}.
# 2) The name of the argument.
# 3) The type of the argument.
#

$typemap_get_ptr{'double'} =	# For double precision, if the input array is
				# of Real type, we can just return a pointer
				# into it.  If it's an integer array, we have
				# to copy it into a temporary array of double.
  sub {
    my ($arg, $argname) = @_;	# Name the arguments.
    my $retstr = "  Matrix _$argname = $arg->{oct_expr}.matrix_value();\n";
				# Get the argument as a matrix.
    if ($arg->{oct_expr_out}) {	# Is this a modify variable?
      $retstr .= ("  _$argname.make_unique();\n" . # Make our modifiable copy.
		  "  $arg->{oct_expr_out} = octave_value(_$argname);\n");
				# Set up the output variable.
    }
    $retstr .= "  $arg->{c_var_name} = &_$argname(0,0);\n";
  };

$typemap_get_ptr{'float'} =	# Floats and ints require a temporary array.
  $typemap_get_ptr{'int'} = $typemap_get_ptr{'unsigned'} =
  $typemap_get_ptr{'short'} = $typemap_get_ptr{'unsigned short'} = sub {
    my ($arg, $argname, $argtype) = @_;	# Name the arguments.
    my $retstr =
      ("  Matrix _$argname = $arg->{oct_expr}.matrix_value();\n" .
       "  $arg->{c_var_name} = ($argtype *)alloca(_$argname.dim1() * _$argname.dim2() * sizeof ($argtype));\n" .
       "  _cvt_double_to($arg->{c_var_name}, &_$argname(0,0), _$argname.dim1()*_$argname.dim2());\n");
    if ($arg->{oct_expr_out}) {	# A modify variable?
      $retstr .= "  $arg->{oct_expr_out} = octave_value(Matrix(_dim(_$argname, 0), _dim(_$argname, 1)));\n";
				# Reserve space for the output.
    }
    $retstr;
  };

$typemap_get_ptr{'complex < double >'} = sub {
  my ($arg, $argname) = @_;	# Name the arguments.
  my $retstr = "  ComplexMatrix _$argname = $arg->{oct_expr}.complex_matrix_value();\n";
  if ($arg->{oct_expr_out}) {	# Modify variable?
    $retstr .= ("  _$argname.make_unique();\n" . # Make a modifiable copy.
		"  $arg->{oct_expr_out} = octave_value(_$argname);\n");
  }
  $retstr .= "  $arg->{c_var_name} = &_$argname(0,0);\n";
};

$typemap_get_ptr{'complex < float >'} = sub {
  my ($arg, $argname) = @_;	# Name the arguments.

  my $retstr =
    ("  ComplexMatrix _$argname = $arg->{oct_expr}.complex_matrix_value();\n" .
     "  $arg->{c_var_name} = (complex<float> *)alloca(_$argname.dim1() * _$argname.dim2() * sizeof (complex<float>));\n" .
     "  _cvt_double_to((float *)$arg->{c_var_name}, (double *)&_$argname(0,0), 2*_$argname.dim1() * _$argname.dim2());\n");
				# Note that this assumes that the internal
				# representation of a complex number is the
				# real part and then the imaginary part.
  if ($arg->{oct_expr_out}) {	# Is this a modify variable?
      $retstr .= "  $arg->{oct_expr_out} = octave_value(ComplexMatrix(_dim($_argname, 0), _dim($argname, 1)));\n";
				# Reserve space for the output.
  }
  $retstr;
};

# $typemap_get_ptr{'char *'} = sub {
#   my ($arg, $argname) = @_;	# Name the arguments.
#
#   if (!defined($string_vector_to_argv_output)) { # Need conversion subroutine?
# 				# Output a function to do it.  We output this
# 				# with print so it goes before the function
# 				# we are currently wrapping.
#     print OUTFILE "
# //
# // Convert a string vector into an array of char * values.  Arguments:
# // 1) The vector of string values.
# // 2) A char ** array which has already been allocated to the correct
# //    dimensions.
# //
# static void
# _string_vector_to_argv(string_vector &strvec, char **c_vec) {
#   for (int idx = 0; idx < strvec.length(); ++idx)
#   {
#     strvec(idx) += '\0';	// Make sure this is null terminated.
#     c_vec[idx] = (char *)strvec(idx).data(); // Copy the pointer.
#   }
# }
# ";
#     $string_vector_to_argv_output = 1; # Don't output it again.
#   }
#
#   my $retstr =
#     ("  string_vector _$argname = $arg->{oct_expr}.all_strings();\n" .
# 				# Get it as a string vector.
#      "  $arg->{c_var_name} = (char **)alloca(_$argname.length() * sizeof (char *));\n" .
#      "  _string_vector_to_argv(_$argname, $arg->{c_var_name});\n");
#   if ($arg->{oct_expr_out}) {	# Is this a modify variable?
#     $retstr .= "  int _${argname}_len = _$argname.length();\n";
# 				# Store the number of entries for later.
#   }
#   $retstr;
# };

$typemap_get_ptr{'char'} = sub {
  my ($arg, $argname) = @_;	# Name the arguments.

  my $retstr = 
    "  charMatrix _$argname = $arg->{oct_expr}.char_matrix_value();\n";
				# Get the value as a matrix.
  if ($arg->{oct_expr_out}) {	# Is this a modify variable?
    $retstr .= "  _$argname.make_unique();\n"; # Make a modifiable copy.
  }
  $retstr .= "  $arg->{c_var_name} = (char *)_$argname.data();\n";
};

#
# Create an output argument vector of a given size, and put a pointer to the
# space into the appropriate C variable.  Arguments:
# 1) The argument description associative array.
# 2) The argument name.
# 3-4) The dimensions.
#
$typemap_output_array_make{'double'} = sub {
  my ($arg, $argname, @dims) = @_; # Name the arguments.

  ("  if ($dims[0] == 1 && $dims[1] == 1)\n" .
   "    $arg->{c_var_name} = (double *)alloca(sizeof (double));\n" .
   "  else\n" .
   "  {\n" .
   "    $arg->{oct_expr_out} = octave_value(Matrix($dims[0], $dims[1]));\n" .
    				# Allocate the space.
   "    $arg->{c_var_name} = (double *)$arg->{oct_expr_out}.matrix_value().data();\n" .
   "  }\n");
};

$typemap_output_array_make{'float'} = # These types require an intermediate
  $typemap_output_array_make{'int'} = # temporary vector.
  $typemap_output_array_make{'unsigned'} = 
  $typemap_output_array_make{'short'} =
  $typemap_output_array_make{'unsigned short'} = sub {
    my ($arg, $argname, @dims) = @_; # Name the arguments.
    ("  if ($dims[0] != 1 || $dims[1] != 1)\n" .
     "    $arg->{oct_expr_out} = octave_value(Matrix($dims[0], $dims[1]));\n" .
      			# Allocate the space.  This also lets us
      			# know the dimension for later.
     "  $arg->{c_var_name} = ($arg->{basic_type} *)alloca($dims[0] * $dims[1] * sizeof ($arg->{basic_type}));\n");
      			# Allocate the temporary array.
  };

# $typemap_output_array_make{'char *'} = sub {
#   my ($arg, $argname, @dims) = @_; # Name the arguments.
#
#   ("  int _${argname}_len = $dims[0]*$dims[1];\n" .	# Save # of entries for later.
#    "  $arg->{c_var_name} = (char *)alloca(_${argname}_len * sizeof (char *));\n");
# 				# Allocate the temporary array.
# };

$typemap_output_array_make{'complex < double >'} = sub {
  my ($arg, $argname, @dims) = @_; # Name the arguments.

  ("  if ($dims[0] == 1 && $dims[1] == 1)\n" .
   "    $arg->{c_var_name} = ($arg->{basic_type} *)alloca(sizeof ($arg->{basic_type}));\n" .
   "  else\n" .
   "  {\n" .
   "    $arg->{oct_expr_out} = octave_value(ComplexMatrix($dims[0], $dims[1]));\n" .
    				# Allocate the space.
   "    $arg->{c_var_name} = (Complex *)$arg->{oct_expr_out}.complex_matrix_value().data();\n" .
   "  }\n");
};


$typemap_output_array_make{'complex < float >'} = sub {
  my ($arg, $argname, @dims) = @_; # Name the arguments.

  ("  if ($dims[0] != 1 || $dims[1] != 1)\n" .
   "    $arg->{oct_expr_out} = octave_value(ComplexMatrix($dims[0], $dims[1]));\n" .
    				# Make a matrix so we remember the size.
   "  $arg->{c_var_name} = ($arg->{basic_type} *)alloca(sizeof ($arg->{basic_type} * $dims[0] * $dims[1]));\n");
				# Make a temporary array.
};


$typemap_output_array_make{'char'} = sub {
				# An array of characters is represented as a
				# CharMatrix object.
  my ($arg, $argname, @dims) = @_; # Name the arguments.

  ("  $arg->{oct_expr_out} = octave_value(charMatrix($dims[0], $dims[1]), true);\n" .
    				# Allocate the space.
   "  $arg->{c_var_name} = (char *)$arg->{oct_expr_out}.char_matrix_value().data();\n");
};

#
# Create a scalar value for output.  Arguments:
# 1) The associative array describing the argument.
# 2) A C expression for the argument.
#
$typemap_output_scalar_make{'double'} =
  $typemap_output_scalar_make{'float'} =
  $typemap_output_scalar_make{'unsigned'} =
  $typemap_output_scalar_make{'int'} =
  $typemap_output_scalar_make{'short'} =
  $typemap_output_scalar_make{'unsigned short'} =
  $typemap_output_scalar_make{'char *'} =
  $typemap_output_scalar_make{'complex < double >'} =
  $typemap_output_scalar_make{'complex < float >'} =
  $typemap_output_scalar_make{'char'} = sub {
    '';				# There's nothing to do.  retval already
				# contains a scalar since that's the way
				# it's initialized, and the only thing we
				# need to do for sure at this point is to
				# make sure that it's a scalar so later on
				# we can test to see if it's a scalar or
				# vector.
  };

#
# Set the value of an output/modify argument which is guaranteed to be a
# scalar.  All functions in this hash are called with three arguments:
# 1) The $function_def{args}{$argname} associative array.
#
$typemap_put_scalar{'double'} = 
  $typemap_put_scalar{'complex < double >'} =
  $typemap_put_scalar{'char *'} = sub {
    my ($arg) = @_;	# Name the arguments.

    "  $arg->{oct_expr_out} = octave_value($arg->{c_var_name});\n";
  };

$typemap_put_scalar{'float'} =
  $typemap_put_scalar{'int'} = $typemap_put_scalar{'unsigned'} =
  $typemap_put_scalar{'short'} = $typemap_put_scalar{'short'} = sub {
    my ($arg) = @_;	# Name the arguments.

    "  $arg->{oct_expr_out} = octave_value((double)$arg->{c_var_name});\n";
  };

$typemap_put_scalar{'complex < float >'} = sub {
  my ($arg) = @_;		# Name the arguments.
  
  ("  $arg->{oct_expr_out} = octave_value(Complex($arg->{c_var_name}.real(),\n" .
   "                                              $arg->{c_var_name}.imag()));\n");
};
  
$typemap_put_scalar{'char'} = sub {
				# Treat a single character as a charMatrix.
  my ($arg) = @_;		# Name the arguments.
  
  ("  $arg->{oct_expr_out} = octave_value(charMatrix(1,1), true);\n" . # Make the matrix.
   "  *(char *)$arg->{oct_expr_out}.char_matrix_value().data() = $arg->{c_var_name};\n");
};

#
# Set the value of an output/modify argument which may be an array.
# The type and dimensions are guaranteed to be proper.
# All functions in this hash are called with three arguments:
# 1) The $function_def{args}{$argname} associative array.
# 2) The name of the argument.
#
$typemap_put_ptr{'double'} =
  $typemap_put_ptr{'complex < double >'} = sub {
    my ($arg) = @_;		# Access the arguments.
    ("  if (_arraylen($arg->{oct_expr_out}) == 1)\n" . # Is this a scalar?
     "    $arg->{oct_expr_out} = octave_value(*$arg->{c_var_name});\n"); # Perform the assignment.
  };

$typemap_put_ptr{'float'} =
  $typemap_put_ptr{'int'} = $typemap_put_ptr{'unsigned'} =
  $typemap_put_ptr{'short'} = $typemap_put_ptr{'unsigned short'} = sub {
    my ($arg) = @_;		# Name the arguments.
    ("  if (_arraylen($arg->{oct_expr_out}) == 1)\n" . # Is this a scalar?
     "    $arg->{oct_expr_out} = octave_value(double(*$arg->{c_var_name}));\n" .
     "  else\n" .
     "    _cvt_to_double($arg->{c_var_name}, (double *)$arg->{oct_expr_out}.matrix_value().data(), _arraylen($arg->{oct_expr_out}));\n");
  };

$typemap_put_ptr{'complex < float >'} = sub {
  my ($arg) = @_;		# Name the arguments.
  ("  if (_arraylen($arg->{oct_expr_out}) == 1)\n" . # Is this a scalar?
   "    $arg->{oct_expr_out} = octave_value(Complex($arg->{c_var_name}\->real(), $arg->{c_var_name}\->imag()));\n" .
   "  else\n" .
   "    _cvt_to_double((float *)$arg->{c_var_name}, (double *)$arg->{oct_expr_out}.matrix_value().data(), 2*_arraylen($arg->{oct_expr_out}));\n");
				# This depends on the fact that complex numbers
				# are stored using their real and imaginary
				# values.
};

# $typemap_put_ptr{'char *'} = sub {
#   my ($arg, $argname) = @_;	# Name the arguments.
#   ("  if (_${argname}_len == 1)\n" .
#    "    $arg->{oct_expr_out} = octave_value(*$arg->{c_var_name});\n" .
#    "  else\n" .
#    "  {\n" .
#    "    string_vector _temp_vec($arg->{c_var_name}, _${argname});\n" .
#    "    $arg->{oct_expr_out} = octave_value(_temp_vec);\n" .
#    "  }\n");
# };

$typemap_put_ptr{'char'} = sub {
  '';				# Nothing to do--pointer already set up.
}

#
# Below this point are the functions that get copied into the header of every
# module:
#
__DATA__

#include "oct.h"

#ifndef __GNUC__
#include <alloca.h>
#endif

inline int max(int a, int b) { return a >= b ? a : b; }
inline int min(int a, int b) { return a >= b ? a : b; }

//
// Function to get the total length (rows*columns) of an octave object of
// arbitrary type.
// Arguments:
// 1) The octave object.
//
// If the object is a scalar, the array length is 1.
//
static int
_arraylen(const octave_value &o_obj)
{
  return max(o_obj.rows(),1) * max(o_obj.columns(),1); // Return the size.
				// max is necessary because sometimes
				// rows() or columns() return -1 or 0 for
				// scalars.
}

//
// Function to get the number of dimensions of an object.
//
static int
_n_dims(const octave_value &o_obj)
{
  if (max(o_obj.columns(),1) > 1)
    return 2;
				// max is necessary because sometimes
				// rows() or columns() return -1 or 0 for
				// scalars.
  else if (max(o_obj.rows(),1) > 1)
    return 1;
  else
    return 0;
}

//
// Return the n'th dimension of an object.  Dimension 0 is the 1st dimension.
//
static inline int
_dim(const octave_value &o_obj, int dim_idx)
{
  if (dim_idx == 0)
    return max(o_obj.rows(), 1);
				// max is necessary because sometimes
				// rows() or columns() return -1 or 0 for
				// scalars.
  else if (dim_idx == 1)
    return max(o_obj.columns(), 1);
  else
    return 1;
}

//
// The following function converts an array of doubles into some other
// numeric type.  Arguments:
// 1) Where to store the result.  The type is determined from the type of
//    this pointer.
// 2) A vector of doubles to convert.
// 3) The number of doubles.
//
template <class FLOAT>
static inline void
_cvt_double_to(FLOAT *out_arr, double *in_arr, unsigned n_el)
{
  while (n_el-- > 0)
    *out_arr++ = (FLOAT)(*in_arr++);
}

template void _cvt_double_to(int *, double *, unsigned);
template void _cvt_double_to(unsigned *, double *, unsigned);
template void _cvt_double_to(short *, double *, unsigned);
template void _cvt_double_to(unsigned short *, double *, unsigned);
template void _cvt_double_to(float *, double *, unsigned);
				// Instantiate our templates.  Octave uses
				// manual template instantiation.

//
// Convert an array of some other type into an array of doubles.  Arguments:
// 1) The array of objects of other type.
// 2) The output array of doubles.
// 3) The number of elements to convert.
//
template <class FLOAT>
static inline void
_cvt_to_double(FLOAT *arr, double *d_arr, unsigned n_el)
{
  while (n_el-- > 0)
    *d_arr++ = double(*arr++);
}

template void _cvt_to_double(int *, double *, unsigned);
template void _cvt_to_double(unsigned *, double *, unsigned);
template void _cvt_to_double(short *, double *, unsigned);
template void _cvt_to_double(unsigned short *, double *, unsigned);
template void _cvt_to_double(float *, double *, unsigned);
				// Instantiate our templates.  Octave uses
				// manual template instantiation.


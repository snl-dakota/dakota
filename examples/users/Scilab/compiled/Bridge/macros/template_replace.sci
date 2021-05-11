function txt = template_replace(template_file_name, keys, values, ...
				out_file_name, comma)

  // argument checking
  
  rhs = argn(2)
  if rhs == 3 then
    out_file_name = ''
    comma = %F
  elseif rhs == 4 then
    comma = %F
  elseif rhs ~= 5 then
    error(39)  // error: incorrect number of arguments
  end
  
  // avoid too small numbers that are not well read by ... filengrene
  values = clean(values);
  
  // load template file into string vector
  template_file = mopen(template_file_name)
  txt = mgetl(template_file)
  mclose(template_file)
  
  // line by line replace the code in the markups <?=code> with their values

  for i = 1 : size(txt, 1)

    // since several markups can be in one line, replacements are done 
    // iteratively, until no markup is found
    
    replacements = %T
    while replacements
      
      // find the position of the first opening markups <?=
      open_markups = strindex(txt(i), "<?=")
      open_markup = open_markups(1)
      
      if open_markup ~= [] then
        
        // find the position of the corresponding closing markup
        close_markups = strindex(txt(i), ">")
        close_markups = close_markups(find(close_markups > open_markup))
        close_markup = close_markups(1)
  
        // extract the code between them
        code = part(txt(i), open_markup+3:close_markup-1)
  
        // replace the var names by their values
        for j = 1:size(keys, 2)
	  code = strsubst(code, keys(j), string(values(j)))
	end
      
        // evaluate the code in scilab shell
        code = string(evstr(code))
	
	// if requested, the decimal points are converted into commas
	if comma then
	  code = strsubst(code, '.', ',')
	end
     
        // replace the result into the line
        txt(i) = part(txt(i), 1:open_markup-1) + code + ...
  	         part(txt(i), close_markup+1: length(txt(i)))

      else
	replacements = %F // exit of the while loop
	
      end
      
    end

  end
  
  // write file  
  if out_file_name ~= '' then
    out_file = mopen(out_file_name, "wb") // does it work on windows ?
    mputl(txt, out_file)
    mclose(out_file)
    txt = ''
  end
  
endfunction

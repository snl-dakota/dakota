function [keyval, err] = parse(filename, key, keytype, keynumber)

//  Notes: 
//  * the file is read by lines, i.e., keys and their values 
//  should be one the same line.
//  * Type checking is not bullet proof but relies
//  on msscanf behavior. For example, if reading a real entered as 
//  "2,4" instead of "2.4", msscanf will read "2." without complaining ...
//  When reading integers, the program will read one integer (wrongly)
//  written with a dot, eg 2. , but it won't read many (which is correct).
//  No type checking possible for strings.
//  * The coma_notation flag in the program changes ',' into '.' after keywords.
//  I don't think it does any harm (if the usual dot notation for numbers 
//  is used) apart from slightly slowing down the routine.
//  If there is no problem with , in numbers and you want a faster execution, 
//  you might want to set it to 0.
//
//  Developper's notes : 
//  * Could have a mode without type checking for faster execution 
//  * Should have a version where reading is not sensitive to line breaks

  coma_notation = 1 // see notes
  
  nb_key_words = size(key, 1)
  [nlhs, nrhs] = argn()
  if nrhs < 2 then
    error('At least 2 arguments needed')
  elseif (nrhs == 2) then
    keynumber = ones(nb_key_words, 1)
    for i = 1:nb_key_words
      keytype(i) = 'real'
    end
  elseif (nrhs == 3) then
    keynumber = ones(nb_key_words, 1)
  end
  abskeynumber = abs(keynumber) // keynumbers can be negative
                                // (i.e. empty fields allowed)
  keylength = length(key)
  keyval = mlist(['keylist', key])
  nb_key = sum(abskeynumber)
  nb_keyfound = 0
  keyfound = zeros(nb_key, 1)
  
  [ifile, err] = mopen(filename, 'r')
//  if err ~>0 then
//    error('\n No data file\n')   
//  end

  if err == 0 then // file to read exists

    while meof(ifile) == 0 & nb_keyfound < nb_key

      txt = mgetl(ifile, 1) // read the file one line at a time
      [ind, wkey] = strindex(txt, key) // ind = position of key in txt

      for nk = 1:length(wkey)
        ik = wkey(nk)
	
        // if the key has already been read, stop

        if keyfound(ik) == 1 then
          error('Key ' + key(ik) + ' already found in file ' + filename)
        end

        // linepart is what remains of the line after key(s)

        linepart = part(txt, [(ind(nk)+keylength(ik)):length(txt)])
        if coma_notation == 1 then
          linepart = strsubst(linepart, ',', '.')
        end

	// parsing reals
	
        if keytype(ik) == 'real' then
          // get ready to read the next abskeynumber(ik) real numbers
          strread = 'msscanf(linepart,'''
          for ir = 1:1:abskeynumber(ik)
            strread = strread + '%e'
          end
          strread = strread + ''')'
          tmp = evstr(strread) // read the abskeynumber(i) next reals
          nb_read = length(tmp)
          if nb_read < abskeynumber(ik) then
            if keynumber(ik) < 0 then
              // fill the rest of tmp with %nan
              tmp([(nb_read+1):abskeynumber(ik)]) = %nan
              tmp = tmp' // somehow the above operation transposes tmp ???
            else
              error('Not enough real numbers after ' + key(ik) + ...
		    ' key in file ' + filename)
            end
          end
          setfield(key(ik), tmp, keyval)
          nb_keyfound = nb_keyfound + abskeynumber(ik)
          keyfound(ik) = 1

	// parsing integers
        
	elseif keytype(ik) == 'integer' then
          // get ready to read the next abskeynumber(ik) integers
          strreadi = 'msscanf(linepart,'''
          strreade = 'msscanf(linepart,'''
          for ir = 1:1:abskeynumber(ik)
            strreadi = strreadi + '%i'
            strreade = strreade + '%e'
          end
          strreadi = strreadi + ''')'
          strreade = strreade + ''')'
          tmpi = evstr(strreadi) // read the abskeynumber(i) next integers
          tmpe = evstr(strreade) // read them as reals
          nb_read = length(tmpi)
          if tmpi ~= tmpe then // a possible test for integers
            error('Not integers after ' + key(ik) + ' key infile ' + ...
		  filename)
          end
          if nb_read < abskeynumber(ik) then
            if keynumber(ik) < 0 then
              tmpi([(nb_read+1):abskeynumber(ik)]) = %nan
              tmpi = tmpi' // somehow the above operation transposes tmp ???
            else
              error('Not enough integers after ' + key(ik) + ...
		    ' key in file ' + filename)
            end
          end
          setfield(key(ik), tmpi, keyval)
          nb_keyfound = nb_keyfound + abskeynumber(ik)
          keyfound(ik) = 1

	// parsing strings
	  
        elseif keytype(ik) == 'string' then
          // get ready to read the next abskeynumber(ik) strings
          strread = 'msscanf(linepart,'''
          for ir = 1:1:abskeynumber(ik)
            strread = strread + '%s'
          end
          strread = strread + ''')'
          tmp = evstr(strread) // read the abskeynumber(i) next strings
          nb_read = size(tmp, 2)
          if nb_read < abskeynumber(ik) then
            if keynumber(ik) < 0 then
              tmp([(nb_read+1):abskeynumber(ik)]) = ''
              tmp = tmp' // somehow the above operation transposes tmp ???
            else
              error('Not enough strings after ' + key(ik) + ' key')
            end
          end
          setfield(key(ik), tmp, keyval)
          nb_keyfound = nb_keyfound + abskeynumber(ik)
          keyfound(ik) = 1

        else
          error('Something wrong with the keytype : ' + keytype(ik))
        end
      end

    end  // end while main file reading loop

    if meof(ifile) ~= 0 & nb_keyfound < nb_key then
      error('EOF hit but all keys have not been found in file' + filename)
    end

    err2 = mclose(ifile)

    for i = 1:nb_key_words
      if keyfound(i) == 0 then
        error('Key ' + key(i) + ' not found in file ' + filename)
      end
    end
  
  end // end if err==0, file exists

endfunction

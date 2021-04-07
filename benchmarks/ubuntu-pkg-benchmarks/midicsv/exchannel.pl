
    #	Extract events for a channel from a MIDI CSV file.
    #	All non-channel events are output unchanged.
    #	Comments are discarded.
    
    $which_channel = 9;     	# Extract General MIDI percussion for demo
    
    while ($a = <>) {
    	if (!($a =~ m/\s*[\#\;]/)) { 	# Ignore comment lines
	    if ($a =~ m/\s*\d+\s*,\s*\d+\s*,\s*\w+_c\s*,\s*(\d+)/) {
    	    	if ($1 == $which_channel) {
		    print($a);
		}
	    } else {
	    	print($a);
	    }
        }
    }

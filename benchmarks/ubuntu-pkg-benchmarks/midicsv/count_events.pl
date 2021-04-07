
    #	Count number of events by type in CSV MIDI file
    #	and report.

    while ($a = <>) {

    	if (!($a =~ m/\s*[\#\;]/)) { 	# Ignore comment lines
	    if ($a =~ m/\d+\s*,\s*\d+\s*,\s*(\w+)/) {
	    	$events{$1}++;
	    } else {
	    	print("Cannot parse: $a");
	    }
        }
    }
    
    foreach $k (sort(keys(%events))) {
    	printf("%9d  %s\n", $events{$k}, $k);
    }


    require 'general_midi.pl';

#   	   Repeats, Note,
#   	      Duration, Velocity
    @track = (4, $GM_Percussion{'Acoustic Bass Drum'},
    	    	  480, 127,
    	      4, $GM_Percussion{'Low-Mid Tom'},
	          240, 127,
	      1, 0, 120, 0,
	      2,  $GM_Percussion{'Hand Clap'},
	      	  240, 127,
	      1, 0, 240, 0
	     );

    print << "EOD";
0, 0, Header, 1, 1, 480
1, 0, Start_track
1, 0, Tempo, 500000
EOD

    $time = 0;
    
    &loop(4, @track);
        
    print << "EOD";
1, $time, End_track
0, 0, End_of_file
EOD

    sub note {	# &note($note_number, $duration [, $velocity])
    	local ($which, $duration, $vel) = @_;
	
	if ($which > 0) {
	    if (!defined($vel)) {
	    	$vel = 127;
	    }
    	    print("1, $time, Note_on_c, 9, $which, $vel\n");
	}
	$time += $duration;
	if ($which > 0) {
    	    print("1, $time, Note_off_c, 9, $which, 0\n");
	}
    }
    
    sub loop {	# &loop($ntimes, @track)	    
    	local ($loops, @tr) = @_;
	local ($i, $r);

    	for ($i = 0; $i < $loops; $i++) {
	    local @t = @tr;
	    while ($#t > 0) {
    		local ($repeats, $note, $duration, $velocity) = splice(@t, 0, 4);
		for ($r = 0; $r < $repeats; $r++) {
		    &note($note, $duration, $velocity);
		}
	    }
	}
    }

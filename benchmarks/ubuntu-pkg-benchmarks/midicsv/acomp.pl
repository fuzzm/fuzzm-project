
    #	Incredibly dumb algorithmic composer

    require 'general_midi.pl';

    $instrument = $GM_Patch{'Distortion Guitar'};
    $tonespan = 32;
    $num_notes = 120;
    $percussion = $GM_Percussion{'Ride Cymbal 1'};
    $beat = 6;

    print << "EOD";
0, 0, Header, 1, 1, 480
1, 0, Start_track
1, 0, Tempo, 500000
1, 0, Program_c, 1, $instrument
EOD

    $time = 0;
    srand(time());
    
    for ($i = 0; $i < $num_notes; $i++) {
    	$n = 60 + int((rand() * $tonespan) - int($tonespan / 2));
    	$notelength = 120 + (60 * int(rand() * 6));
	&note(1, $n, $notelength, 127);
	if (($i % $beat) == 0) {
	    print("1, $time, Note_on_c, 9, $percussion, 127\n");
	} elsif (($i % $beat) == ($beat - 1)) {
	    print("1, $time, Note_off_c, 9, $percussion, 0\n");
	}
    }
    
    #	Cymbal crash at end
    $cymbal = $GM_Percussion{'Crash Cymbal 2'};
    print("1, $time, Note_on_c, 9, $cymbal, 127\n");
    $time += 480;
    print("1, $time, Note_off_c, 9, $cymbal, 0\n");
    
    #	Audience applause
    $time += 480;
    print("1, $time, Program_c, 1, $GM_Patch{'Applause'}\n");     
    print("1, $time, Note_on_c, 1, 60, 100\n");
    for ($i = 16; $i <= 32; $i++) {
    	$time += 120;
	$v = int(127 * ($i / 32));
	print("1, $time, Poly_aftertouch_c, 1, 60, $v\n");
    }
    for ($i = 32; $i >= 0; $i--) {
    	$time += 240;
	$v = int(127 * ($i / 32));
	print("1, $time, Poly_aftertouch_c, 1, 60, $v\n");
    }
    print("1, $time, Note_off_c, 1, 60, 0\n");
   
    print << "EOD";
1, $time, End_track
0, 0, End_of_file
EOD

    sub note {	# &note($channel, $note_number, $duration [, $velocity])
    	local ($channel, $which, $duration, $vel) = @_;
	
	if (!defined($vel)) {
	    $vel = 127;
	}
    	print("1, $time, Note_on_c, $channel, $which, $vel\n");
	$time += $duration;
    	print("1, $time, Note_off_c, $channel, $which, 0\n");
    }

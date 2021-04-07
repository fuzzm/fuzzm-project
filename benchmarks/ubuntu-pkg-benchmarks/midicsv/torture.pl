
#   	Generate "torture test" for MidiCSV / CSVmidi

#   The output is a CSV file written on standard output
#   which should be fed to through CSVMIDI and MIDICSV
#   and the output compared.

    srand(1234);    	    # Make pseudorandom sequences repeatable

    print << 'EOD';
    
;
  ;   MIDIcsv Torture Test
    	#
0, 0, Header, 1, 4, 480
1, 0, Start_track
1, 0, TiTlE_t, "MidiCSV Torture Test"
1,0,	copyRight_t, "© 1808 L. van Beethoven.  This document is in the public domain."
1, 0, SMPTE_offset, 96, 0, 0, 0, 0
1, 0, key_signature, 0, minor
1, 0, Time_signature, 4, 2, 24, 8
1, 100, Text_t, "The tempo varies somewhat in this passage."
1, 482, Tempo, 335977
1, 961, Tempo, 333493
1, 1442, Tempo, 333319
1, 10578, End_track
2, 0, Start_track
2, 0, Sequence_number, 64000
2, 0, MIDI_port, 0
2, 0, Instrument_Name_t, "Brass Section"
2, 0, Program_c, 1, 61
2, 259, marker_T, "Bom bom bom..."
2, 259, Lyric_t, "Bom"
2, 259, Note_on_c, 1, 67, 104
2, 501, Note_off_c, 1, 67, 0
2, 525, Lyric_t, "Bom"
2, 525, Note_on_c, 1, 67, 104
2, 740, Note_on_c, 1, 67, 0
2, 764, Lyric_t, "Bom"
2, 764, Note_on_c, 1, 67, 104
2, 978, Note_on_c, 1, 67, 0
2, 1000, Marker_t, "...BOM!"
2, 1003, Lyric_t, "BOMMMMM!"
2, 1003, Note_on_c, 1, 63, 104
2, 4809, Note_on_c, 1, 63, 0
    #   Second 4 notes
2, 4850, Cue_point_t, "Conductor falls off podium"
2, 5047, Instrument_name_t, """Rock"" Organ"
2, 5048, Program_c, 1, 18
2, 5048, Note_on_c, 1, 65, 104
2, 5100, pitch_bend_c, 1, 10000
2, 5100, channel_aftertouch_c, 1, 127
2, 5200, pitch_bend_c, 1, 4000
2, 5289, note_off_c, 1, 65, 0
2, 5300, Control_c, 1, 91, 120
2,5300, Control_c, 1, 7, 40
2, 5311, Note_on_c, 1, 65, 104
2,5400,pitch_BEND_c,1,0
2, 5526, Note_on_c, 1, 65, 0
2, 5540, control_c, 1, 7, 127
2,5540, control_c, 1, 10, 0
2, 5549, Pitch_Bend_c, 1,8192
2, 5549, Note_on_c, 1, 65, 104
2, 5766, Note_on_c, 1, 65, 0
2,5790, control_c, 1, 10, 127
2, 5790, Note_on_c, 1, 62, 104
2, 9000, Poly_aftertouch_c, 1, 62, 127
2, 10578, Note_off_c, 1, 62, 0
2, 10578, End_track
3, 0, Start_track
3, 10578, End_track
4, 0, Start_Track
4, 10, Channel_prefix, 1
4, 400, System_exclusive, 10, 0, 255, 127, 30, 96, 255, 224, 108, 31, 0
4, 510, System_exclusive, 4, 0, 121, 31, 19
4, 1071, channel_prefix, 15
4, 1071, System_exclusive_packet, 10, 255, 18, 33, 111, 0, 77, 201, 7, 4, 255
4, 1076, System_exclusive_packet, 0
4, 1108, System_exclusive_packet, 3, 0, 0, 0
4, 2000, Sequencer_specific, 10, 80, 211, 54, 71, 229, 0, 13, 128, 12, 40
4, 2020, Sequencer_specific, 0
4, 3000, Unknown_meta_event, 121, 9, 39, 201, 118, 6, 91, 223, 0, 78, 56
;   The following event is actually
   ;    4, 3030, Key_signature, -4, "major"
#   coded as an Unknown_meta_event.
4, 3100, Unknown_meta_event, 89, 2, 252, 0
EOD

    #	Now programmatically generate some long strings
    #	and byte vectors to push the limits.
    
    #	Moderately long Sysex
    $n = 5123;
    print("4, 4000, System_exclusive, $n");
    &obytes($n);
    print("\n");
    
    #	Moderately long pure ASCII text string
    $n = 11213;
    print("4, 4100, Text_t, \"");
    for ($i = 0; $i < $n; $i ++) {
    	$r = chr(ord(' ') + int(rand(127 - ord(' '))));
	if ($r eq '"') {
	    print('"');
	} elsif ($r eq '\\') {
	    print('\\');
	}
    	print("$r");
    }
    print("\"\n");
    
    #	Rather long arbitrary text string
    print("4, 4200, Text_t, ");
    &ostring(74219);
    print("\n");
    
    #	Really long Sequence_specific
    $n = 3497861;   	# 250,000th prime!
    print("4, 4300, Sequencer_specific, $n");
    &obytes($n);
    print("\n");
    
    #	Really long arbitrary text string
    print("4, 4400, Lyric_t, ");
    &ostring(4256233);	# 300,000th prime!
    print("\n");    
    
    #	Wind up the track and the file with canned data

    print << 'EOD';
4,10500, End_track
0, 0, End_of_file
EOD

    #	Generate a random character string containing all
    #	byte values from 0 through 255.  The argument gives
    #	the length in bytes to generate.

    sub ostring
    {
    	local ($howlong) = @_;
	local ($i, $r);
	
	print('"');
	for ($i = 0; $i < $howlong; $i ++) {
    	    $r = chr(int(rand(256)));
	    if ($r eq '"') {
		print('"');
	    } elsif ($r eq '\\') {
		print('\\');
	    } elsif ((ord($r) < ord(' ')) ||
	    	     ((ord($r) >= 127) && (ord($r) <= 160))) {
		printf("\\%03o", ord($r));
		next;
    	    }
    	    print("$r");
        }
	print('"');
    }

    #	Generate a byte sequence whose length is
    #	given by the argument.
    
    sub obytes
    {
    	local ($howlong) = @_;
	local ($i, $r);

	for ($i = 0; $i < $howlong; $i ++) {
    	    $r = int(rand(256));
    	    print(", $r");
	}
    }

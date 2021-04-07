
    #   Chorus all notes in a CSV MIDI file

    $offset = -12;
    $percussion = 9;

    while ($a = <>) {
        print($a);

        #   Recognise Note_on_c and Note_off_c records and crack into:

        #       $1  Start of record
        #       $2  Channel number
        #       $3  Note number
        #       $a  Balance of record

        if ($a =~ s/(\d+,\s*\d+,\s*Note_\w+,\s*(\d+),\s*)(\d+)//) {
            if ($2 != $percussion) {
                $n = $3;
                $n += $offset;
                if ($n < 0) {
                    next;
                }
                $a = "$1$n$a";
                print($a);
            }
        }
    }

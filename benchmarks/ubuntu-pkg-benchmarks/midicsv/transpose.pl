
    #   Transpose all notes in a CSV MIDI file

    #   This Perl program is an example of how simple it can
    #   be to transform MIDI files in CSV format.  This program
    #   filters a CSV MIDI file from standard input to standard
    #   output, shifting all notes by the value given as
    #   $offset.  Notes on the $percussion channel are not
    #   shifted.

    $offset = -12;
    $percussion = 9;

    while ($a = <>) {

        #   Recognise Note_on_c and Note_off_c records and crack into:

        #       $1  Start of record
        #       $2  Channel number
        #       $3  Note number
        #       $a  Balance of record

        if ($a =~ s/(\d+,\s*\d+,\s*Note_\w+,\s*(\d+),\s*)(\d+)//) {
            $n = $3;
            if ($2 != $percussion) {
                $n += $offset;
            }
            if ($n < 0) {
                next;
            }
            $a = "$1$n$a";
        }
        print($a);
    }

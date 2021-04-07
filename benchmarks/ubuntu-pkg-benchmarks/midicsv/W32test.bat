@Rem    Test release versions of Midicsv and Csvmidi with
@Rem    an identity transform.

Release\Midicsv test.mid w.csv
Release\Csvmidi w.csv w.mid
Rem  The following comparison should not find any differences
cmp test.mid w.mid

Command line used to find this crash:

./afl-fuzz -m8000M -i programs/3318c71ea11c4a759fa406bd5dec2038245d6b47c55c50b5127368d31949c6a3-wasm/test-case -o programs/3318c71ea11c4a759fa406bd5dec2038245d6b47c55c50b5127368d31949c6a3-wasm/findings -- programs/3318c71ea11c4a759fa406bd5dec2038245d6b47c55c50b5127368d31949c6a3-wasm/prog.wasm.instr @@

If you can't reproduce a bug outside of afl-fuzz, be sure to set the same
memory limit. The limit used for this fuzzing session was 7.81 GB.

Need a tool to minimize test cases before investigating the crashes or sending
them to a vendor? Check out the afl-tmin that comes with the fuzzer!

Found any cool bugs in open-source tools using afl-fuzz? If yes, please drop
me a mail at <lcamtuf@coredump.cx> once the issues are fixed - I'd love to
add your finds to the gallery at:

  http://lcamtuf.coredump.cx/afl/

Thanks :-)

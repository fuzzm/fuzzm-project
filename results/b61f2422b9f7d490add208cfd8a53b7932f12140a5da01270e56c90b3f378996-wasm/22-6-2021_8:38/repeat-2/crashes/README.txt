Command line used to find this crash:

./afl-fuzz -m8000M -i programs/b61f2422b9f7d490add208cfd8a53b7932f12140a5da01270e56c90b3f378996-wasm/test-case -o programs/b61f2422b9f7d490add208cfd8a53b7932f12140a5da01270e56c90b3f378996-wasm/findings -- programs/b61f2422b9f7d490add208cfd8a53b7932f12140a5da01270e56c90b3f378996-wasm/prog.wasm.instr -f @@

If you can't reproduce a bug outside of afl-fuzz, be sure to set the same
memory limit. The limit used for this fuzzing session was 7.81 GB.

Need a tool to minimize test cases before investigating the crashes or sending
them to a vendor? Check out the afl-tmin that comes with the fuzzer!

Found any cool bugs in open-source tools using afl-fuzz? If yes, please drop
me a mail at <lcamtuf@coredump.cx> once the issues are fixed - I'd love to
add your finds to the gallery at:

  http://lcamtuf.coredump.cx/afl/

Thanks :-)

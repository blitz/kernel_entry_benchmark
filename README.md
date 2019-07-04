[![Build Status](https://travis-ci.org/blitz/kernel_entry_benchmark.svg?branch=master)](https://travis-ci.org/blitz/kernel_entry_benchmark)
![GitHub](https://img.shields.io/github/license/blitz/kernel_entry_benchmark.svg?color=green)

# Kernel Entry Microbenchmarks

Find the full description in my blog post about [x86 system call
mechanisms](https://x86.lol/generic/2019/07/04/kernel-entry.html) over on
[x86.lol](https://x86.lol/).

This repository contains x86 kernel entry benchmarks for:

 - interrupt gates,
 - call gates,
 - `sysenter`/`sysexit`,
 - `syscall`/`sysret`.

For building and running them you need:

 - gcc >= 7.x
 - [tup](http://gittup.org/tup/)
 - qemu
 - python3 with matplotlib

If you've installed all these, you can run the benchmarks and generate an SVG
plot by running `tup`.

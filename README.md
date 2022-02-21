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

## Building with Nix (Recommended)

If you have [Nix](https://nixos.org/), you can build the measurement
kernel like this:

```sh
% nix-build ./ci.nix -A defaultPackage
```

The kernel will then be `result` in the current directory.

If you have Nix with
[Flakes](https://nixos.wiki/wiki/Flakes#Non-NixOS) (still not enabled
by default), this can be shortened to:

```sh
% nix build
```

## Building without Nix

For building and running without Nix you need:

 - gcc >= 7.x
 - make

Then you can just invoke make to build the kernel:

```sh
% make -C src
```

Then you can find the kernel as `src/kernel_entry_benchmark.elf32`.

Whether this works is up to your system. Only the Nix build is tested.

## Running Benchmarks

The tiny benchmark kernel assumes that it runs on Qemu and can use the
Qemu debug console. You can run it as a Multiboot-compatible kernel:

```sh
% qemu-system-x86_64 -kernel result -debugcon stdio  -no-reboot -enable-kvm
Interrupt Gate,610
Call Gate,473
Syscall,99
```

The output is in CSV format. The first column is the kernel entry
method and the second is the length of a roundtrip to the kernel using
this method in cycles.

I leave implementing serial output and running it on real hardware as
an exercise to the reader.

**Note:** Only Intel CPUs work currently. There seems to be an issue
on AMD CPUs that currently limit this benchmark from running there.

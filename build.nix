{ stdenv, gnumake, nasm }:

stdenv.mkDerivation {
  name = "kernel-entry-benchmark";
  src = ./src;
  nativeBuildInputs = [ gnumake nasm ];

  installPhase = ''
    runHook preInstall

    install -m 444 kernel_entry_benchmark.elf32 $out

    runHook postInstall
  '';
}

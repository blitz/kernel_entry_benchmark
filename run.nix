{ stdenvNoCC, qemu, util-linux, lib, makeWrapper
, kernel-entry-benchmark }:

stdenvNoCC.mkDerivation {
  name = "benchmark-run";

  src = ./scripts;

  nativeBuildInputs = [ makeWrapper ];
  buildInputs = [ qemu ];

  dontConfigure = true;
  dontBuild = true;

  installPhase = ''
    runHook preInstall

    mkdir -p $out/bin

    patchShebangs run
    install -m 0755 run $out/bin/benchmark-run
    wrapProgram $out/bin/benchmark-run --prefix PATH : ${lib.makeBinPath [ qemu util-linux ]} \
      --add-flags '${kernel-entry-benchmark}'

    runHook postInstall
  '';
}

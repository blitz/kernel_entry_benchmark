{ stdenvNoCC, python3, lib, makeWrapper }:

stdenvNoCC.mkDerivation {
  name = "benchmark-run";

  src = ./scripts;

  nativeBuildInputs = [ makeWrapper ];
  buildInputs = [ (python3.withPackages (ps: [ ps.matplotlib ])) ];

  dontConfigure = true;
  dontBuild = true;

  installPhase = ''
    runHook preInstall

    mkdir -p $out/bin

    patchShebangs plot
    install -m 0755 plot $out/bin/benchmark-plot

    runHook postInstall
  '';
}

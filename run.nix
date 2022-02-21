{ stdenvNoCC, qemu, python3, lib, makeWrapper }:

stdenvNoCC.mkDerivation {
  name = "benchmark-run";

  src = ./scripts;

  nativeBuildInputs = [ makeWrapper ];
  buildInputs = [ qemu (python3.withPackages (ps: [ ps.matplotlib ])) ];

  dontConfigure = true;
  dontBuild = true;

  installPhase = ''
    runHook preInstall

    mkdir -p $out/bin

    patchShebangs run
    install -m 0755 run $out/bin/benchmark-run
    wrapProgram $out/bin/benchmark-run --prefix PATH : ${lib.makeBinPath [ qemu ]}

    patchShebangs plot
    install -m 0755 plot $out/bin/benchmark-plot

    runHook postInstall
  '';
}

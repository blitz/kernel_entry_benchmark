{ runCommand, benchmark, benchmark-run }:
runCommand "benchmark-result" {
  nativeBuildInputs = [ benchmark-run ];
  requiredSystemFeatures = [ "kvm" ];
} ''

  echo $PATH
  mkdir -p $out
  benchmark-run "${benchmark}" | tee $out/benchmark.csv

  if [ -z "$(cat $out/benchmark.csv)" ]; then
    echo "VM produced no output?"
    exit 1
  fi

  cat $out/benchmark.csv | benchmark-plot $out/benchmark.png
''

{
  description = "A very basic flake";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-21.11";
        flake-compat = {
      url = "github:edolstra/flake-compat";
      flake = false;
    };
    flake-compat-ci.url = "github:hercules-ci/flake-compat-ci";
  };

  outputs = { self, nixpkgs, flake-compat, flake-compat-ci }: {

    packages.x86_64-linux =
      let
        callPackage = nixpkgs.legacyPackages.x86_64-linux.callPackage;
      in rec {
        kernel-entry-benchmark = callPackage ./build.nix {};

        benchmark-plot = callPackage ./plot.nix {};
        benchmark-run = callPackage ./run.nix {
          inherit kernel-entry-benchmark;
        };

        benchmark-results = callPackage ./benchmark.nix {
          inherit benchmark-run benchmark-plot;
        };
      };

    defaultPackage.x86_64-linux = self.packages.x86_64-linux.kernel-entry-benchmark;

    # For Hercules CI, which doesn't natively support flakes (yet).
    ciNix = flake-compat-ci.lib.recurseIntoFlakeWith {
      flake = self;

      # Optional. Systems for which to perform CI.
      systems = [ "x86_64-linux" ];
    };
  };
}

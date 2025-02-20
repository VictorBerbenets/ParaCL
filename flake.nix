{
  description = "The set of dependencies to build the paraCL project";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    flake-parts.url = "github:hercules-ci/flake-parts";
    treefmt-nix = {
      url = "github:numtide/treefmt-nix";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs =
    { flake-parts, treefmt-nix, ... }@inputs:
    flake-parts.lib.mkFlake { inherit inputs; } {
      imports = [ treefmt-nix.flakeModule ];

      systems = [
        "x86_64-linux"
        "aarch64-linux"
      ];

      perSystem =
        { pkgs, ... }:
        let
          llvmPackages = pkgs.llvmPackages_19;
        in
        rec {
          packages = rec {
            paracl = pkgs.callPackage ./. { stdenv = llvmPackages.stdenv; };
            default = paracl;
          };

          devShells.default = (pkgs.mkShell.override { stdenv = llvmPackages.stdenv; }) {
            nativeBuildInputs =
              packages.paracl.nativeBuildInputs
              ++ (with pkgs; [
                valgrind
                lldb
                gtest
                flex
                bison
              ]);
            buildInputs = packages.paracl.buildInputs;
          };
        };
    };
}

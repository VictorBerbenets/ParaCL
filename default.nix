{ pkgs, stdenv, ... }:
stdenv.mkDerivation {
  src = ./.;
  pname = "paraCL (custom C language)";
  version = "0.1.0";
  nativeBuildInputs = with pkgs; [
    flex
    bison
    cmake
    lit
    filecheck
    gersemi
    graphviz  
  ];
  buildInputs = [pkgs.llvm_19];
}

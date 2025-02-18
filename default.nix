let
   pkgs = import <nixpkgs> {};
   llvmPackages = pkgs.llvmPackages_19;
   llvm = llvmPackages.llvm;
 in
 pkgs.mkShell {
   buildInputs = [ llvm ];
   nativeBuildInputs = [ pkgs.cmake pkgs.gtest pkgs.bison pkgs.flex
                         pkgs.clang-tools pkgs.lit pkgs.valgrind];
}

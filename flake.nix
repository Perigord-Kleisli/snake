{
  description = "A very basic flake";

  outputs = {
    self,
    nixpkgs,
  }: let
    system = "x86_64-linux";
    pkgs = import nixpkgs {inherit system;};
  in {
    devShells.${system}.default = pkgs.mkShell {
      EM_CONFIG = pkgs.writeText ".emscripten" ''
        EMSCRIPTEN_ROOT = '${pkgs.emscripten}/share/emscripten'
        LLVM_ROOT = '${pkgs.emscripten.llvmEnv}/bin'
        BINARYEN_ROOT = '${pkgs.binaryen}'
        NODE_JS = '${pkgs.nodejs-18_x}/bin/node'
        CACHE = '/home/truff/.local/src/snake/.cache'
      '';
      buildInputs = with pkgs; [
        cmake-language-server
        clang-tools
        lldb
        cmake
        ninja
        pkg-config
        SDL2
        SDL2_mixer
        cppcheck
        emscripten
        glibc
      ];
      GLIBC_DEV = "${pkgs.glibc.dev}";
    };
  };
}

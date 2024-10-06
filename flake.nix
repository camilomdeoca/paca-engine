{
  description = "A Nix-flake-based Node.js development environment";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-24.05";
  };

  outputs = inputs@{ flake-parts, ... }:
    flake-parts.lib.mkFlake { inherit inputs; } {
      imports = [ ];
      systems = [ "x86_64-linux" ];
      perSystem = { config, self', inputs', pkgs, system, ... }: {
        devShells.default = pkgs.mkShell {
          buildInputs = with pkgs; [
            clang
            libclang
            llvm
            libllvm
            gdb
            gf
            valgrind
            gcc14
            git
            lazygit
            SDL2
            glew
            cmake
            assimp
            glm
            libpng
            pngpp
            freetype
          ];
          shellHook = ''
            export CXX=${pkgs.gcc14}/bin/g++
          '';
        };
      };
      flake = {
      };
    };
}

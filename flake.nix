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
          hardeningDisable = [ "fortify" ];
          buildInputs = with pkgs; [
            llvmPackages_18.clang-tools    # libclang from LLVM 1
            llvmPackages_18.libclang    # libclang from LLVM 1
            llvmPackages_18.libllvm    # libclang from LLVM 1
            #gcc14Stdenv
            gdb
            gf
            renderdoc
            valgrind
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

            # For editor
            qt6.full
          ];
          shellHook = ''
            export CXX="${pkgs.gcc14}/bin/g++"
            CPLUS_INCLUDE_PATH+=":${pkgs.gcc14.cc}/include/c++/14.1.0"
            CPLUS_INCLUDE_PATH+=":${pkgs.glibc.dev}/include"
            CPLUS_INCLUDE_PATH+=":${pkgs.gcc14.cc}/include/c++/14.1.0/x86_64-unknown-linux-gnu"
            CPLUS_INCLUDE_PATH+=":${pkgs.gcc14.cc}/lib/gcc/x86_64-unknown-linux-gnu/14.1.0/include"
            CPLUS_INCLUDE_PATH+=":${pkgs.glm}/include"
            CPLUS_INCLUDE_PATH+=":${pkgs.qt6.full}/include"
            export CPLUS_INCLUDE_PATH

            export QT_QPA_PLATFORM=xcb
          '';
            #export CPATH="${pkgs.gcc14Stdenv.cc.libc_dev}/include:${pkgs.gcc14Stdenv.cc.cc}/include/c++/14.0.1"
            #export CXXFLAGS="-I${pkgs.gcc14Stdenv.cc.libc_dev}/include"
            #export LDFLAGS="-L${pkgs.gcc14Stdenv.cc.libc}/lib"
        };
      };
      flake = {
      };
    };
}

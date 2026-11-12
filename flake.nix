{
  description = "metatron devenv";

  inputs = {
    tsssni.url = "github:tsssni/tsssni.nix";
    nixpkgs.follows = "tsssni/nixpkgs";
    systems.follows = "tsssni/systems";
    flake-parts.follows = "tsssni/flake-parts";
  };

  outputs =
    {
      flake-parts,
      ...
    }@inputs:
    flake-parts.lib.mkFlake { inherit inputs; } {
      systems = import inputs.systems;

      perSystem =
        {
          pkgs,
          lib,
          system,
          ...
        }:
        rec {
          _module.args.pkgs = import inputs.nixpkgs {
            inherit system;
            overlays = with inputs; [ tsssni.overlays.default ];
          };

          packages.default = pkgs.callPackage ./nix { };

          devShells.default = pkgs.mkShell.override { stdenv = pkgs.clangStdenv; } {
            inputsFrom = [ packages.default ];
            packages =
              with pkgs;
              [
                clang-tools
                cmake-language-server
              ]
              ++ lib.optionals pkgs.stdenv.hostPlatform.isLinux [
                hotspot
                perf
              ];
            shellHook = ''
              export CMAKE_INSTALL_PREFIX=$HOME/metatron/out
            ''
            + lib.optionalString pkgs.stdenv.hostPlatform.isLinux ''
              export VK_LAYER_PATH=${pkgs.vulkan-validation-layers}/share/vulkan/explicit_layer.d
            ''
            + lib.optionalString pkgs.stdenv.hostPlatform.isDarwin ''
              export MTL_DEBUG_LAYER=1
            '';
          };
        };
    };
}

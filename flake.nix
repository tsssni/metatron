{
  description = "metatron devenv";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    tsssni = {
      url = "github:tsssni/tsssni.nix";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs =
    {
      nixpkgs,
      tsssni,
      ...
    }:
    let
      lib = nixpkgs.lib;

      systems = [
        "aarch64-darwin"
        "x86_64-linux"
        "aarch64-linux"
      ];

      systemAttrs = f: system: { ${system} = f system; };

      mapSystems = f: systems |> lib.map (systemAttrs f) |> lib.mergeAttrsList;

      packages = mapSystems (
        system:
        let
          pkgs = import nixpkgs {
            inherit system;
            overlays = tsssni.pkgs;
          };
        in
        {
          default = pkgs.callPackage ./nix { };
        }
      );

      devShells = mapSystems (
        system:
        let
          pkgs = import nixpkgs {
            inherit system;
            overlays = tsssni.pkgs;
          };
        in
        {
          default = pkgs.mkShell.override { stdenv = pkgs.clangStdenv; } {
            inputsFrom = [ packages.${system}.default ];
            shellHook = "
              export CMAKE_INSTALL_PREFIX=$HOME/metatron/out
              export SHELL=nu
            " + lib.optionalString pkgs.stdenv.isLinux ''
              export VK_LAYER_PATH=${pkgs.vulkan-validation-layers}/share/vulkan/explicit_layer.d
            '';
          };
        }
      );
    in
    {
      inherit packages devShells;
    };
}

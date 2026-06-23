{
  description = "metatron devenv";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    tsssni = {
      url = "github:tsssni/tsssni.nix";
      inputs.nixpkgs.follows = "nixpkgs";
    };
    nixgl = {
      url = "github:tsssni/nixGL";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs =
    {
      nixpkgs,
      tsssni,
      nixgl,
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
            config.allowUnfree = true;
          };
          glpkgs = import nixgl { inherit pkgs; };
        in
        rec {
          default = pkgs.mkShell.override { stdenv = pkgs.clangStdenv; } {
            inputsFrom = [ packages.${system}.default ];
            packages = with pkgs; [
              clang-tools
              cmake-language-server
            ] ++ lib.optionals pkgs.stdenv.isLinux [
              hotspot
              perf
            ];
            shellHook = ''
              export CMAKE_INSTALL_PREFIX=$HOME/metatron/out
            ''
            + lib.optionalString pkgs.stdenv.isLinux ''
              export VK_LAYER_PATH=${pkgs.vulkan-validation-layers}/share/vulkan/explicit_layer.d
            ''
            + lib.optionalString pkgs.stdenv.isDarwin ''
              export MTL_DEBUG_LAYER=1
            '';
          };

          impure = default.overrideAttrs (oldAttrs: {
            nativeBuildInputs =
              oldAttrs.nativeBuildInputs
              ++ (with glpkgs; [
                nixVulkanIntel
                auto.nixVulkanNvidia
              ]);
          });
        }
      );
    in
    {
      inherit packages devShells;
    };
}

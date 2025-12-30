{
  description = "metatron devenv";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    tsssni = {
      url = "github:tsssni/tsssni.nix";
      inputs.nixpkgs.follows = "nixpkgs";
    };
    nixgl = {
      url = "github:nix-community/nixGL";
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
          clangdOverlay = (
            final: prev:
            let
              patchClangTools =
                llvmPkgSet:
                llvmPkgSet.overrideScope (
                  lfinal: lprev: {
                    clang-tools = lprev.clang-tools.overrideAttrs (oldAttrs: {
                      postInstall = (oldAttrs.postInstall or "") + ''
                        echo "Applying PR 462747 patch to the generated clangd wrapper..."
                        patch -i ${
                          prev.fetchpatch {
                            url = "https://github.com/NixOS/nixpkgs/pull/462747.diff";
                            hash = "sha256-WDP/WJAflkYw8YQDgXK3q9G7/Z6BXTu7QpNGAKjO3co=";
                          }
                        } $out/bin/clangd
                      '';
                    });
                  }
                );
            in
            { llvmPackages = patchClangTools prev.llvmPackages; }
          );
          pkgs = import nixpkgs {
            inherit system;
            overlays = tsssni.pkgs ++ [ clangdOverlay ];
            config.allowUnfree = true;
          };
          glpkgs =
            let
              isx86 = system == "x86_64-linux";
            in
            import nixgl {
              inherit pkgs nvidiaVersion;
              enable32bits = isx86;
              enableIntelX86Extensions = isx86;
            };
          externalNvidiaVersion = builtins.getEnv "MTT_IMPURE_NVIDIA_VERSION";
          nvidiaVersion = if externalNvidiaVersion == "" then null else externalNvidiaVersion;
        in
        rec {
          default = pkgs.mkShell.override { stdenv = pkgs.clangStdenv; } {
            inputsFrom = [ packages.${system}.default ];
            packages = with pkgs; [
              clang-tools
              cmake-language-server
              vulkan-validation-layers
            ];
            shellHook = ''
              export CMAKE_INSTALL_PREFIX=$HOME/metatron/out
              export SHELL=nu
            ''
            + lib.optionalString pkgs.stdenv.isLinux ''
              export VK_LAYER_PATH=${pkgs.vulkan-validation-layers}/share/vulkan/explicit_layer.d
            '';
          };

          impure = default.overrideAttrs (oldAttrs: {
            nativeBuildInputs =
              oldAttrs.nativeBuildInputs
              ++ (with glpkgs; [
                nixVulkanIntel
                nixVulkanNvidia
              ]);
          });
        }
      );
    in
    {
      inherit packages devShells;
    };
}

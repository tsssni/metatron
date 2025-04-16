{
	description = "metatron devenv";

	inputs = {
		nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
	};

	outputs = {
		nixpkgs
		, ...
	}:
	let
		lib = nixpkgs.lib;

		systems = [
			"aarch64-darwin"
			"x86_64-linux"
		];

		mapSystems = f: systems
			|> lib.map f
			|> lib.mergeAttrsList;

		devShells = mapSystems (system:
			let
				pkgs = import nixpkgs {
					inherit system;
				};
			in {
				"${system}".default = pkgs.mkShellNoCC {
					packages = with pkgs; []
					# toolchain
					++ [
						gcc
						lldb
						cmake
						ninja
					]
					# external
					++ [
						mimalloc
						openimageio
						(openvdb.overrideAttrs (old: rec {
							version = "12.0.1";
							src = fetchFromGitHub {
								owner = "AcademySoftwareFoundation";
								repo = "openvdb";
								rev = "v${version}";
								sha256 = "sha256-ofVhwULBDzjA+bfhkW12tgTMnFB/Mku2P2jDm74rutY=";
							};
						}))
					];
				};
			}
		);

		packages = mapSystems (system:
			let
				pkgs = import nixpkgs {
					inherit system;
				};
			in {
				"${system}".default = pkgs.callPackage ./nix {};
			}
		);

	in { 
		inherit devShells packages;
	};
}

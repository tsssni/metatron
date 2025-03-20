{
	description = "metatron engine devenv";

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

		devShells = systems
			|> lib.map (system:
				let
					pkgs = import nixpkgs {
						inherit system;
					};
				in {
					"${system}".default = pkgs.mkShellNoCC {
						packages = with pkgs; [
							gcc
							lldb
							cmake
							ninja
						];
					};
				}
			)
			|> lib.mergeAttrsList;
	in { inherit devShells; };
}

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
		setupShell = shell: system: let
			pkgs = import nixpkgs {
				inherit system;
			};
		in pkgs.mkShell {
			packages = with pkgs; [
				gcc
				lldb
				cmake
				ninja
			];

			shellHook = ''
				${shell}
			'';
		};
	in {
		devShells."aarch64-darwin".default = setupShell "zsh" "aarch64-darwin";
		devShells."x86_64-linux".default = setupShell "elvish" "x86_64-linux";
	};
}

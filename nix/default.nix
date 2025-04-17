{
  lib
, stdenv
, fetchFromGitHub
, clang-tools
, cmake
, ninja
, mimalloc
, openimageio
, openvdb
}:
stdenv.mkDerivation {
	pname = "metatron";
	version = "dev";

	src = fetchFromGitHub {
		owner = "tsssni";
		repo = "metatron";
		rev = "fdfabc6";
		sha256 = "sha256-TdtEh1sOWcRy8ZG1LQcr/h7NqQ9wh8x4mOoCrMdqrKQ=";
	};

	nativeBuildInputs = [
		clang-tools
		cmake
		ninja
	];

	buildInputs = [
		mimalloc
		openimageio
		openvdb
	];

	cmakeFlags = [
		"--preset rel"
	];

	buildPhase = ''
		cd ..
		cmake --build build/rel
	'';

	installPhase = ''
		cd build/rel
		cmake --install . --prefix $out
	'';
	
	meta = with lib; {
		description = "physically-based path-tracing renderer";
		homepage = "github.com/tsssni/metatron";
		license = licenses.gpl3;
		platforms = platforms.linux ++ platforms.darwin;
	};
}

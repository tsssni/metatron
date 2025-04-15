{
  lib
, stdenv
, fetchFromGitHub
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
		rev = "ab973d3";
		sha256 = "sha256-slDc/3jE3lepF59Cr0E9DG/OEVVHvC9bCQmKSvoHZqE=";
	};

	nativeBuildInputs = [
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
		mkdir -p $out
		cp -r build/rel/* $out/
	'';
	
	meta = with lib; {
		description = "physically-based path-tracing renderer";
		homepage = "github.com/tsssni/metatron";
		license = licenses.gpl3;
		platforms = platforms.linux ++ platforms.darwin;
	};
}

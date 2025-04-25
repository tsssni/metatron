{
  lib
, stdenv
, fetchFromGitHub
, clang-tools
, cmake
, ninja
, mimalloc
, openimageio
, zlib
, openvdb
}:
stdenv.mkDerivation {
	pname = "metatron";
	version = "dev";

	src = fetchFromGitHub {
		owner = "tsssni";
		repo = "metatron";
		rev = "3a38056";
		sha256 = "sha256-CzkMkes+pJPQLFuuY3MMrR54ZSG3ZnEUBjx7qXB4Xp4=";
	};

	nativeBuildInputs = [
		clang-tools
		cmake
		ninja
	];

	buildInputs = [
		mimalloc
		zlib
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

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

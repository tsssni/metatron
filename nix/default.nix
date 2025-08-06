{
  argparse,
  assimp,
  clangStdenv,
  clang-tools,
  cmake,
  entt,
  fetchFromGitHub,
  glaze,
  lib,
  mimalloc,
  ninja,
  openimageio,
  openvdb,
  proxy,
  zlib,
}:
clangStdenv.mkDerivation {
  pname = "metatron";
  version = "dev";

  src = fetchFromGitHub {
    owner = "tsssni";
    repo = "metatron";
    rev = "b8f4ec1";
    sha256 = "sha256-ekcc30V8CMY4/got2xnU4VfoE3huDwoJQWNzQ6W4Zag=";
  };

  nativeBuildInputs = [
    clang-tools
    cmake
    ninja
  ];

  buildInputs = [
    argparse
    assimp
    entt
    glaze
    mimalloc
    openimageio
    openvdb
    proxy
    zlib
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

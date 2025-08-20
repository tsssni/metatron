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
  version = "0.0.1";

  src = fetchFromGitHub {
    owner = "tsssni";
    repo = "metatron";
    rev = "575a754";
    sha256 = "sha256-zp7IurWr7rmUcmGZggNDDJ5V4GBt+Olw68ocJGw24UU=";
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

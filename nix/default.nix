{
  lib,
  clangStdenv,
  fetchFromGitHub,
  clang-tools,
  cmake,
  ninja,
  mimalloc,
  assimp,
  openimageio,
  openvdb,
  proxy,
  zlib,
  entt,
}:
clangStdenv.mkDerivation {
  pname = "metatron";
  version = "dev";

  src = fetchFromGitHub {
    owner = "tsssni";
    repo = "metatron";
    rev = "13af11b";
    sha256 = "sha256-ZLfrMO7aMMY4a9k8INcsFYJQlM3gp8z7E7hhsjhcDXo=";
  };

  nativeBuildInputs = [
    clang-tools
    cmake
    ninja
  ];

  buildInputs = [
    mimalloc
    assimp
    openimageio
    openvdb
    proxy
    zlib
    entt
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

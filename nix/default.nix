{
  argparse,
  assimp,
  clangStdenv,
  clang-tools,
  cmake,
  entt,
  glaze,
  lib,
  mimalloc,
  ninja,
  openimageio,
  openvdb,
  proxy,
  vulkan-tools,
  zlib,
}:
clangStdenv.mkDerivation {
  pname = "metatron";
  version = "0.1.0";

  src = ../.;
  # src = fetchFromGitHub {
  #   owner = "tsssni";
  #   repo = "metatron";
  #   rev = "";
  #   sha256 = "";
  # };

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
  ] ++ (lib.optionals clangStdenv.isLinux [
    vulkan-tools
  ]);

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
    platforms = [
        "x86_64-linux"
        "aarch64-linux"
        "aarch64-darwin"
    ];
  };
}

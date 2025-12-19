{
  apple-sdk_15,
  argparse,
  assimp,
  clangStdenv,
  cmake,
  glaze,
  lib,
  ninja,
  openimageio,
  openvdb,
  proxy,
  shader-slang,
  spirv-cross,
  vulkan-loader,
  vulkan-headers,
  vulkan-tools,
  zlib,
}:
let
  stdenv = clangStdenv;
in
stdenv.mkDerivation {
  pname = "metatron";
  version = "0.2.0";

  src = ../.;
  # src = fetchFromGitHub {
  #   owner = "tsssni";
  #   repo = "metatron";
  #   rev = "";
  #   sha256 = "";
  # };

  nativeBuildInputs = [
    cmake
    ninja
    shader-slang
    spirv-cross
  ];

  buildInputs = [
    argparse
    assimp
    glaze
    openimageio
    openvdb
    proxy
    zlib
  ]
  ++ (lib.optionals stdenv.isLinux [
    vulkan-loader
    vulkan-headers
    vulkan-tools
  ])
  ++ (lib.optionals stdenv.isDarwin [
    apple-sdk_15
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

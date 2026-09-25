{
  argparse,
  assimp,
  clangStdenv,
  cmake,
  cpptrace,
  glaze,
  lib,
  metal-cpp,
  ninja,
  openimageio,
  openvdb,
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
  version = "0.3.0";

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
  ];

  buildInputs = [
    argparse
    assimp
    cpptrace
    (glaze.override {
      enableSSL = false;
      enableInterop = false;
    })
    openimageio
    openvdb
    zlib
  ]
  ++ (lib.optionals stdenv.hostPlatform.isLinux [
    vulkan-loader
    vulkan-headers
    vulkan-tools
  ])
  ++ (lib.optionals stdenv.hostPlatform.isDarwin [
    metal-cpp
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

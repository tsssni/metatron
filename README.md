![dispersion](https://github.com/tsssni/metatron-scenes/raw/master/dispersion/dispersion.png)
Metatron
[<img src="https://builtwithnix.org/badge.svg" height="20">](https://builtwithnix.org)
[<img src="https://github.com/tsssni/metatron/actions/workflows/ci.yml/badge.svg?branch=master" height="20">](https://github.com/tsssni/metatron/actions/workflows/ci.yml)
========

## Introduction

Metatron is a physically based renderer unbiasedly simulating radiative transfer equation and spectral transport with Metal/Vulkan acceleration.

## Resources

* Demo scenes at [metatron-scenes](https://github.com/tsssni/metatron-scenes).
* Scene exporter at [metatron-exporter](https://github.com/tsssni/metatron-exporter).

## Features

* Spectrum
  * Sampled spectra are used for rendering.
  * RGB compatibility via [J19](https://jo.dreggn.org/home/2019_sigmoid.pdf).
  * [Real world spectra](https://github.com/tsssni/metatron/tree/master/share/spectra) are used for conductor, dispersion and color space definition.
* Medium
  * Null-scattering integral for heterogenous medium via [MG19](https://cs.dartmouth.edu/~wjarosz/publications/miller19null.html).
  * Phase function evaluation and sampling via [HG41](https://adsabs.harvard.edu/full/1941ApJ....93...70H).
  * [NanoVDB](https://www.openvdb.org/documentation/doxygen/NanoVDB_MainPage.html) is supported for volume data.
* BSDF
  * Unified physical BSDF for diffuse, dielectric, conductor and plastic surface via [TS67](https://www.graphics.cornell.edu/~westin/pubs/TorranceSparrowJOSA1967.pdf).
  * Microfacet model and importance sampling via [TR75](https://pharr.org/matt/blog/images/average-irregularity-representation-of-a-rough-surface-for-ray-reflection.pdf) and [H18](https://jcgt.org/published/0007/04/01/paper.pdf).
* Shape
  * Sphere as convenient bounding volume.
  * Mesh in various formats supported by [assimp](https://github.com/assimp/assimp).
* Light
  * Delta light including parallel, point and spot light.
  * Environment light with image importance sampling.
  * Area light for shapes with emissive material. Spherical triangle sampling via [A95](https://www.graphics.cornell.edu/pubs/1995/Arv95c.pdf).
  * Atomosphere with alien world support via [HW12](https://cgg.mff.cuni.cz/projects/SkylightModelling/HosekWilkie_SkylightModel_SIGGRAPH2012_Preprint_lowres.pdf) and [HW13](https://cgg.mff.cuni.cz/publications/adding-a-solar-radiance-function-to-the-hosek-wilkie-skylight-model/). TGMM sky sampling via [VV21](https://diglib.eg.org/items/b3f1efca-1d13-44d0-ad60-741c4abe3d21).
* Sampler
  * Heitz sampler for fast convergence via [HB19](https://eheitzresearch.wordpress.com/762-2/)
  * Z Sobol sampler for visual quality via [AW20](https://repository.kaust.edu.sa/items/1269ae24-2596-400b-a839-e54486033a93)
* Integrator
  * Unbiased ReSTIR PT with pairwise MIS via [LK22](https://graphics.cs.utah.edu/research/projects/gris/)
  * Metal/Vulkan acceleration thanks to hardware ray tracing and [slang](https://github.com/shader-slang/slang).
  * Remote preview of rendering intermediates via [tev](https://github.com/Tom94/tev)

## Build

Metatron use [nix](https://nixos.org) with [flakes](https://nix.dev/concepts/flakes.html) for package management on Linux and Darwin.

```nu
nix build
```

Or use [CMake](https://cmake.org/) with manual [dependencies](https://github.com/tsssni/metatron/blob/master/nix/default.nix#L35). Add prefix flag for runtime resource loading.

```nu
cmake --preset rel -DCMAKE_INSTALL_PREFIX=/usr/bin/ -DCMAKE_CXX_FLAGS="-march=native"
cmake --build build/rel --target install
```

## Usage

Metatron can be used as a library. Use `package` output in [flake.nix](https://github.com/tsssni/metatron/blob/master/flake.nix) or copy [package.nix](https://github.com/tsssni/metatron/blob/master/nix/default.nix) to add metatron.

```nix
# flake.nix
{
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    metatron.url = "github:tsssni/metatron";
  };
  outputs = {nixpkgs, metatron, ...}: let
    pkgs = import nixpkgs {
      system = "x86_64-linux";
      overlays = [
        (final: prev: {
          metatron = metatron.packages.default;
        })
      ];
    };
  in {
    # use pkgs.metatron somewhere
  };
}
```

Metatron will be linked as a shared library.

```cmake
find_package(metatron REQUIRED)
target_link_libraries(renderer PUBLIC metatron)
```

## Run

Run `metatron-tracer -h` for option documents.

```nu
metatron-tracer -s ~/metatron-scenes/classroom/ -o classroom.exr -a localhost:14158 -d gpu
```

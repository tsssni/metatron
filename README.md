![dispersion](https://github.com/tsssni/metatron-scenes/raw/master/dispersion/dispersion.png)
Metatron
[<img src="https://builtwithnix.org/badge.svg" height="20">](https://builtwithnix.org)
[<img src="https://img.shields.io/endpoint.svg?url=https%3A%2F%2Fgarnix.io%2Fapi%2Fbadges%2Ftsssni%2Fmetatron%3Fbranch%3Dmaster" height="18">](https://garnix.io/repo/tsssni/metatron)
========

## Introduction

Metatron is a physically based renderer unbiasedly simulating radiative transfer equation and spectral transport.

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
* Integrator
  * LBVH with parallel construction via [PL10](https://research.nvidia.com/sites/default/files/pubs/2010-06_HLBVH-Hierarchical-LBVH/HLBVH-final.pdf).
  * Direct lighting and spectral MIS inspired by [pbrt-v4](https://pbr-book.org/4ed/Light_Transport_II_Volume_Rendering/Volume_Scattering_Integrators#ImprovingtheSamplingTechniques).

## Build

Metatron use [nix](https://nixos.org) with [flakes](https://nix.dev/concepts/flakes.html) to build the renderer and manage dependencies on Linux and Darwin.

```nu
nix build
```

Or use [cmake](https://cmake.org/) if dependencies are availabe in environment. Dependencies are declared at [`buildInputs`](https://github.com/tsssni/metatron/blob/master/nix/default.nix#L35).

```nu
cmake -B build
cmake --preset rel
cmake --build build/rel
```

Metatron can be used as a library. Use `package` output in [flake.nix](https://github.com/tsssni/metatron/blob/master/flake.nix) or copy [package.nix](https://github.com/tsssni/metatron/blob/master/nix/default.nix) to add metatron package.

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

Or manually install metatron to environment.

```nu
cmake --install build/rel --prefix /usr
```

Modules in [src](https://github.com/tsssni/metatron/tree/master/src) are all optional.

```cmake
find_package(metatron-core REQUIRED)
find_package(metatron-resource REQUIRED)
```

## Usage

Scene directory and output image path in exr format could be provided, or `./` and `./result.exr` will be used.

```nu
metatron-tracer -s ~/metatron-scenes/classroom/ -o classroom.exr
```

Remote preview with [tev](https://github.com/Tom94/tev) is supported.

```nu
metatron-tracer -a localhost:14158
```

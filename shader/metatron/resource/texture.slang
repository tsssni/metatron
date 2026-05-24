module texture;
__include texture.checkerboard;
__include texture.constant;
__include texture.image;
import color;
import shape;
import spectra;
import metatron.core;

#define SPEC_V(method, ...) \
         if (idx.type() == 0) return tag<Constant_Spectrum_Texture>(idx).get().method(__VA_ARGS__); \
    else if (idx.type() == 1) return tag<Image_Spectrum_Texture>(idx).get().method(__VA_ARGS__); \
    else if (idx.type() == 2) return tag<Checkerboard_Texture>(idx).get().method(__VA_ARGS__);

#define SPEC_R(method, ...) SPEC_V(method, __VA_ARGS__) else return {};

#define SPEC_OP(...) \
         if (idx.type() == 0) return tag<Constant_Spectrum_Texture>(idx).get()(__VA_ARGS__); \
    else if (idx.type() == 1) return tag<Image_Spectrum_Texture>(idx).get()(__VA_ARGS__); \
    else if (idx.type() == 2) return tag<Checkerboard_Texture>(idx).get()(__VA_ARGS__); \
    else return {};

#define VEC_V(method, ...) \
         if (idx.type() == 0) return tag<Constant_Vector_Texture>(idx).get().method(__VA_ARGS__); \
    else if (idx.type() == 1) return tag<Image_Vector_Texture>(idx).get().method(__VA_ARGS__);

#define VEC_R(method, ...) VEC_V(method, __VA_ARGS__) else return {};

#define VEC_OP(...) \
         if (idx.type() == 0) return tag<Constant_Vector_Texture>(idx).get()(__VA_ARGS__); \
    else if (idx.type() == 1) return tag<Image_Vector_Texture>(idx).get()(__VA_ARGS__); \
    else return {};

namespace mtt::texture {
    public struct Spectrum_Texture {
        tag<byte> idx;
        public fv4 operator()(Coordinate coord, fv4 spec) { SPEC_OP(coord, spec) }
        public func sample(math::Context ctx, fv2 u) -> fv2 { SPEC_R(sample, ctx, u) }
        public func pdf(fv2 uv) -> f32 { SPEC_R(pdf, uv) }
        public func empty() -> bool { return idx.empty(); }
    }

    public struct Vector_Texture {
        tag<byte> idx;
        public fv4 operator()(Coordinate coord) { VEC_OP(coord) }
        public func sample(math::Context ctx, fv2 u) -> fv2 { VEC_R(sample, ctx, u) }
        public func pdf(fv2 uv) -> f32 { VEC_R(pdf, uv) }
        public func empty() -> bool { return idx.empty(); }
    }

    public struct Coordinate {
        public fv2 uv = {};
        public f32 dudx = {0.f};
        public f32 dudy = {0.f};
        public f32 dvdx = {0.f};
        public f32 dvdy = {0.f};
    }

    public func grad(
        math::Ray_Differential diff,
        shape::Interaction intr
    ) -> Optional<Coordinate> {
        let tangent = math::Plane(intr.p, intr.n);
        let dt_opt = math::hit(diff.r, tangent); if (dt_opt == none) return none;
        let dxt_opt = math::hit(diff.rx, tangent); if (dxt_opt == none) return none;
        let dyt_opt = math::hit(diff.ry, tangent); if (dyt_opt == none) return none;
        let dt = dt_opt.value; let dxt = dxt_opt.value; let dyt = dyt_opt.value;

        let p = diff.r.o + dt * diff.r.d;
        let dpdx = diff.rx.o + dxt * diff.rx.d - p;
        let dpdy = diff.ry.o + dyt * diff.ry.d - p;
        let dpduv = transpose(fm23(intr.dpdu, intr.dpdv));
        let duvdx = math::least_squares(dpduv, dpdx);
        let duvdy = math::least_squares(dpduv, dpdy);

        return Coordinate(intr.uv, duvdx[0], duvdy[0], duvdx[1], duvdy[1]);
    }
}

#undef SPEC_V
#undef SPEC_R
#undef SPEC_OP
#undef VEC_V
#undef VEC_R
#undef VEC_OP

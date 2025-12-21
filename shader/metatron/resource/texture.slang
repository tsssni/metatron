module texture;
__include texture.checkerboard;
__include texture.constant;
__include texture.image;
import shape;
import spectra;
import metatron.core;

namespace mtt::texture {
    public interface Spectrum_Texture {
        fv4 operator()(Coordinate coord, fv4 spec);
        func sample(math::Context ctx, fv2 u) -> fv2;
        func pdf(fv2 uv) -> f32;
    }

    public interface Vector_Texture {
        fv4 operator()(Coordinate coord);
        func sample(math::Context ctx, fv2 u) -> fv2;
        func pdf(fv2 uv) -> f32;
    }

    public struct Spectrum_Texture_Tag {
        tag<byte> idx;

        public func get() -> Spectrum_Texture {
            if (idx.type() == 0) return tag<Constant_Spectrum_Texture>(idx).get();
            else if (idx.type() == 1) return tag<Image_Spectrum_Texture>(idx).get();
            else if (idx.type() == 2) return tag<Checkerboard_Texture>(idx).get();
            else return {};
        }
    }

    public struct Vector_Texture_Tag {
        tag<byte> idx;

        public func get() -> Vector_Texture {
            if (idx.type() == 0) return tag<Constant_Vector_Texture>(idx).get();
            else if (idx.type() == 1) return tag<Image_Vector_Texture>(idx).get();
            else return {};
        }
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

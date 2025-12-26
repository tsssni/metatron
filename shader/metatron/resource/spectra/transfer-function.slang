implementing spectra;

namespace mtt::spectra {
    public interface Transfer_Function {
        public func transfer(f32 x) -> f32;
        public func linearize(f32 x) -> f32;
    }

    public struct sRGB_Transfer_Function: Transfer_Function {
        public func transfer(f32 x) -> f32 {
            if (x <= 0.0031308f) return 12.92f * x;
            else return 1.055f * pow(x, 1.f / 2.4f) - 0.055f;
        }

        public func linearize(f32 x) -> f32 {
            if (x <= 0.04045f) return x / 12.92f;
            else return pow((x + 0.055f) / 1.055f, 2.4f);
        }
    }

    public struct Transfer_Function_Tag {
        tag<byte> idx;

        public func get() -> Transfer_Function {
            if (idx.index() == 0) return sRGB_Transfer_Function();
            else return {};
        }
    }
}

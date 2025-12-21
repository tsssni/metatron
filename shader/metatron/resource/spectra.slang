module spectra;
__include spectra.blackbody;
__include spectra.constant;
__include spectra.color_space;
__include spectra.discrete;
__include spectra.ops;
__include spectra.rgb;
__include spectra.transfer_function;
__include spectra.visible;
import metatron.core;

namespace mtt::spectra {
    public static fv2 visible_lambda = {360.f, 830.f};
    public static f32 CIE_Y_integral = 106.7502593994140625f;

    public interface Spectrum {
        public f32 operator()(f32 lambda);
    }

    public struct Spectrum_Tag {
        tag<byte> idx = {};

        public func get() -> Spectrum {
            if (idx.type() == 0) return tag<Constant_Spectrum>(idx).get();
            else if (idx.type() == 1) return tag<Rgb_Spectrum>(idx).get();
            else if (idx.type() == 2) return tag<Blackbody_Spectrum>(idx).get();
            else if (idx.type() == 3) return tag<Visible_Spectrum>(idx).get();
            else if (idx.type() == 4) return tag<Discrete_Spectrum>(idx).get();
            else return {};
        }

        public func empty() -> bool { return idx.empty(); }
    }
}

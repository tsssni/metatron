module spectra;
__include spectra.blackbody;
__include spectra.constant;
__include spectra.discrete;
__include spectra.ops;
__include spectra.rgb;
__include spectra.stochastic;
__include spectra.visible;
import color;
import metatron.core;

#define DISPATCH_OP(...) \
         if (idx.type() == 0) return tag<Constant_Spectrum>(idx).get()(__VA_ARGS__); \
    else if (idx.type() == 1) return tag<Rgb_Spectrum>(idx).get()(__VA_ARGS__); \
    else if (idx.type() == 2) return tag<Blackbody_Spectrum>(idx).get()(__VA_ARGS__); \
    else if (idx.type() == 3) return tag<Visible_Spectrum>(idx).get()(__VA_ARGS__); \
    else if (idx.type() == 4) return tag<Discrete_Spectrum>(idx).get()(__VA_ARGS__); \
    else return {};

namespace mtt::spectra {
    public static fv2 visible_lambda = {360.f, 830.f};
    public static f32 CIE_Y_integral = 106.7502593994140625f;

    public struct Spectrum {
        tag<byte> idx = {};
        public f32 operator()(f32 lambda) { DISPATCH_OP(lambda) }
        public func empty() -> bool { return idx.empty(); }
    }
}

#undef DISPATCH_OP

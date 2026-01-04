implementing math;

namespace mtt::math {
    public func erf(f32 x) -> f32 {
        let c0 = estrin<f32, 6>(sqr(x), {
            0x1.20dd74p+0, -0x1.812672p-2,
            0x1.ce0934p-4, -0x1.b5a334p-6,
            0x1.4246b4p-8, -0x1.273facp-11,
        });

        let c1 = estrin<f32, 7>(abs(x), {
            -0x1.a0d71ap+0, -0x1.d51e3ap-1,
            -0x1.3a904cp-3,  0x1.1c395cp-5,
            -0x1.6856bep-8,  0x1.180f1ep-11,
            -0x1.8ca9f6p-16,
        });

        let xb = 1 - exp2(c1 * abs(x));
        return select(abs(x) < 1, x * c0, copysign(select(isinf(xb), xb, 1.0), x));
    }

    public func erfinv<T: __BuiltinFloatingPointType>(T x) -> T {
        let w = -log((T(1) - x) * (T(1) + x));

        let w1 = T(w) - T(2.5);
        let w2 = T(math::sqrt(w)) - T(3);

        let p1 = T(polynomial<T, 9>(w1, {
            T(1.50140941),     T(0.246640727),
            T(-0.00417768164), T(-0.00125372503),
            T(0.00021858087), T(-4.39150654e-06),
            T(-3.5233877e-06),  T(3.43273939e-07),
            T(2.81022636e-08),
        }));

        let p2 = T(polynomial<T, 9>(w2, {
            T(2.83297682),     T(1.00167406),
            T(0.00943887047), T(-0.0076224613),
            T(0.00573950773), T(-0.00367342844),
            T(0.00134934322),  T(0.000100950558),
            T(-0.000200214257),
        }));

        return (w < T(5) ? p1 : p2) * x;
    }

    public func gaussian<T: __BuiltinFloatingPointType>(T x, T mu, T sigma) -> T {
        return exp(-sqr(x - mu) / (T(2) * sqr(sigma))) / (sqrt(T(2 * pi)) * sigma);
    }

    public func gaussian_cdf(f32 x, f32 mu, f32 sigma) -> f32 {
        return f32(0.5) * (f32(1) + erf((x - mu) / sigma / sqrt(2)));
    }
}

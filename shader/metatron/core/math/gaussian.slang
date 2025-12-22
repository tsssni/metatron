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

    public func erf(f64 x) -> f64 {
        let c0 = estrin<f64, 12>(sqr(x), {
            0x1.20dd750429b6dp+0,  -0x1.812746b0379bcp-2,
            0x1.ce2f21a040d12p-4,  -0x1.b82ce311fa924p-6,
            0x1.565bccf92b298p-8,  -0x1.c02db03dd71b8p-11,
            0x1.f9a2baa8fee07p-14, -0x1.f4ca4d6f3e31bp-17,
            0x1.b97fd3d992af4p-20, -0x1.5c0726f04e805p-23,
            0x1.d71b0f1b15b0ap-27, -0x1.abae491c540bp-31
        });

        /// max = 4.0 ulp, avg = 0.605 ulp on [1, 6]
        let c1 = estrin<f64, 8>(abs(x), {
            -0x1.a0be83b09c3d7p+0, -0x1.8bb29648c7afep+1,
            -0x1.639eb89a5975p+1,  -0x1.7b48b8cd14d9fp+0,
            -0x1.fb25a03ddc781p-2, -0x1.9cdb7dcacdfb3p-4,
            -0x1.64f7fbe544f07p-7, -0x1.9a3c3874b3919p-12
        }) / estrin<f64, 8>(abs(x), {
            0x1p+0,                 0x1.55b5d06f1c2dep+0,
            0x1.b998ffae5528ep-1,   0x1.46884d56cb49bp-2,
            0x1.18e8848a2cc38p-4,   0x1.ee7f90e8d480cp-8,
            0x1.1c6a194029df4p-12, -0x1.03d1306b29028p-31,
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

    public func gaussian_cdf(f64 x, f64 mu, f64 sigma) -> f64 {
        return f64(0.5) * (f64(1) + erf((x - mu) / sigma / sqrt(f64(2))));
    }
}

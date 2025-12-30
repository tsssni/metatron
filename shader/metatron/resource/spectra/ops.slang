implementing spectra;

namespace mtt {
    using namespace spectra;

    public f32 operator*<T: Spectrum>(T x, T y) {
        var integral = 0.f;
        for (var lambda = visible_lambda[0]; lambda <= visible_lambda[1]; ++lambda)
            integral += x(lambda) * y(lambda);
        return integral;
    }

    public fv4 operator&<T: Spectrum>(fv4 lambda, T s) {
        if (math::constant(lambda)) return s(lambda[0]);
        var value: fv4;
        for (var i = 0; i < 4; ++i) value[i] = s(lambda[i]);
        return value;
    }
}

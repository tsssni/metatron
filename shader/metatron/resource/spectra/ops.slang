implementing spectra;

namespace mtt {
    using namespace spectra;

    public f32 operator*(tag<Spectrum> x, tag<Spectrum> y) {
        var integral = 0.f;
        let z = x.get();
        let w = y.get();
        for (var lambda = visible_lambda[0]; lambda <= visible_lambda[1]; ++lambda)
            integral += z(lambda) * w(lambda);
        return integral;
    }

    public fv4 operator&(fv4 lambda, spectra::Spectrum s) {
        if (math::constant(lambda)) return s(lambda[0]);
        var value: fv4;
        for (var i = 0; i < 4; ++i) value[i] = s(lambda[i]);
        return value;
    }

}

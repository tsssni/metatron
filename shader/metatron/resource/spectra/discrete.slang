implementing spectra;

namespace mtt::spectra {
    public struct Discrete_Spectrum: Spectrum {
        buf<f32> lambda;
        buf<f32> storage;

        public f32 operator()(f32 wavelength) {
            if (lambda.size() == 1) return storage[0];
            let size = i32(lambda.size());
            wavelength = clamp(wavelength, lambda[0], lambda[size - 1]);

            var idx = 0;
            for (; idx < size && lambda[idx] < wavelength; ++idx);
            idx = clamp(idx - 1, 0, size - 2);

            let alpha = 1.f
            * (wavelength - lambda[idx])
            / (lambda[idx + 1] - lambda[idx]);
            return lerp(storage[idx], storage[idx + 1], alpha);
        }
    };
}

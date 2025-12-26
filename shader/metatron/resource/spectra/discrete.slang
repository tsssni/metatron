implementing spectra;

namespace mtt::spectra {
    public struct Discrete_Spectrum: Spectrum {
        buf<f32> lambda;
        buf<f32> storage;

        public f32 operator()(f32 wavelength) {
            if (lambda.size() == 1) return storage[0];
            let size = i32(lambda.size());
            wavelength = clamp(wavelength, lambda[0], lambda[size - 1]);

            var left = 0;
            var right = size;
            while (left < right) {
                let mid = (left + right) / 2;
                if (lambda[mid] < wavelength) left = mid + 1;
                else right = mid;
            }
            let idx = clamp(left - 1, 0, size - 2);

            let alpha = 1.f
            * (wavelength - lambda[idx])
            / (lambda[idx + 1] - lambda[idx]);
            return lerp(storage[idx], storage[idx + 1], alpha);
        }
    }
}

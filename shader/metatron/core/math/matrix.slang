implementing math;

namespace mtt::math {
    public func inverse<T: __BuiltinFloatingPointType, let h: u32>(
        matrix<T, h, h> m
    ) -> matrix<T, h, h> {
        var augmented: matrix<T, h, h * 2>;
        for (var i = 0; i < h; ++i)
            for (var j = 0; j < h; ++j)
                augmented[i][j] = m[i][j];
        for (var i = 0; i < h; ++i)
            augmented[i][h + i] = T(1);

        // Gaussian-Jordan
        for (var i = 0; i < h; ++i) {
            var pivot_row = i;
            var max_val = abs(augmented[i][i]);

            for (var j = i + 1; j < h; ++j) {
                let curr_val = abs(augmented[j][i]);
                if (curr_val > max_val) {
                    max_val = curr_val;
                    pivot_row = j;
                }
            }

            if (pivot_row != i) swap(augmented[i], augmented[pivot_row]);
            let pivot = augmented[i][i];
            augmented[i] /= pivot;

            for (var j = 0uz; j < h; ++j)
                if (j != i) {
                    let factor = augmented[j][i];
                    augmented[j] -= factor * augmented[i];
                }
        }

        var result: matrix<T, h, h>;
        for (var i = 0; i < h; ++i)
            for (var j = 0; j < h; ++j)
                result[i][j] = augmented[i][h + j];
        return result;
    }

    public func least_squares<T: __BuiltinFloatingPointType, let h: u32, let w: u32>(
        matrix<T, h, w> a, vector<T, h> b
    ) -> vector<T, w> {
        let a_t = transpose(a);
        return mul(inverse(mul(a_t, a)), mul(a_t, b));
    }
}

implementing math;

namespace mtt::math {
    public func estrin<T: __BuiltinFloatingPointType, let n: u32>(
        T x, Array<T, n> c
    ) -> T {
        let n_rec = (n - 1) / 2;
        let n_fma = n / 2;
        var c_rec: Array<T, (n - 1) / 2 + 1>;
        for (var i = 0; i < n_fma; ++i)
            c_rec[i] = fma(x, c[2 * i + 1], c[2 * i]);

        if (n_rec == n_fma) c_rec[n_rec] = c[n - 1];
        if (n_rec == 0) return c_rec[0];
        else return estrin<T, (n - 1) / 2 + 1>(math::sqr(x), c_rec);
    }

    public func polynomial<T: __BuiltinFloatingPointType, let n: u32>(
        T x, Array<T, n> c
    ) -> T { return estrin(x, c); }
}

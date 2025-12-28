implementing math;

namespace mtt::math {
    public func constant<T: __BuiltinType, let size: u32>(
        vector<T, size> x
    ) -> bool {
        return all(x == vector<T, size>(x[0]));
    }

    public func min<T: IComparable, let size: u32>(vector<T, size>  v) -> T {
        var result = v[0];
        for (int i = 1; i < size; ++i)
            result = result < v[i] ? result : v[i];
        return result;
    }

    public func max<T: IComparable, let size: u32>(vector<T, size> v) -> T {
        var result = v[0];
        for (int i = 1; i < size; ++i) result = result > v[i] ? result : v[i];
        return result;
    }

    public func minvi<T: IComparable, let size: u32>(vector<T, size> x) -> Tuple<T, u32> {
        var y = x[0];
        var z = 0;
        for (var i = 1; i < size; ++i)
            if (x[i] < y) {
                y = x[i];
                z = i;
            }
        return {y, z};
    }

    public func maxvi<T: IComparable, let size: u32>(vector<T, size> x) -> Tuple<T, u32> {
        var y = x[0];
        var z = 0;
        for (var i = 1; i < size; ++i)
            if (x[i] > y) {
                y = x[i];
                z = i;
            }
        return {y, z};
    }

    public func mini<T: IComparable, let size: u32>(vector<T, size> x) -> u32 {
        return minvi(x)._1;
    }

    public func maxi<T: IComparable, let size: u32>(vector<T, size> x) -> u32 {
        return maxvi(x)._1;
    }

    public func sum<T: IArithmetic, let size: u32>(vector<T, size> x) -> T {
        var y = x[0];
        for (var i = 1; i < size; ++i)
            y = y + x[i];
        return y;
    }

    public func prod<T: IArithmetic, let size: u32>(vector<T, size> x) -> T {
        var y = x[0];
        for (var i = 1; i < size; ++i)
            y = y * x[i];
        return y;
    }

    public func avg<T: IArithmetic, let size: u32>(vector<T, size> x) -> T {
        return sum(x) / T(size);
    }

    public func angle<T: __BuiltinFloatingPointType, let size: u32>(
        vector<T, size> x, vector<T, size> y
    ) -> T {
        // compute theta / 2 to avoid round-off error
        if (dot(x, y) < T(0)) return T(pi) - T(2) * asin(length(-y - x) / T(2));
        else return T(2) * asin(length(y - x) / T(2));
    }

    public func gram_schmidt<T: __BuiltinFloatingPointType, let size: u32>(
        vector<T, size> y, vector<T, size> x
    ) -> vector<T, size> {
        return y - x * dot(x, y);
    }
}

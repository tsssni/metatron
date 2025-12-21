implementing math;

namespace mtt::math {
    public func min<T: IComparable, let n: int>(vector<T, n>  v) -> T {
        var result = v[0];
        for (int i = 1; i < n; ++i)
            result = result < v[i] ? result : v[i];
        return result;
    }

    public func max<T: IComparable, let n: int>(vector<T, n> v) -> T {
        var result = v[0];
        for (int i = 1; i < n; ++i)
            result = result > v[i] ? result : v[i];
        return result;
    }

    public func minvi<T: IComparable, let n: int>(vector<T, n> x) -> Tuple<T, u32> {
        var y = x[0];
        var z = 0;
        for (var i = 1; i < n; ++i)
            if (x[i] < y) {
                y = x[i];
                z = i;
            }
        return {y, z};
    }

    public func maxvi<T: IComparable, let n: int>(vector<T, n> x) -> Tuple<T, u32> {
        var y = x[0];
        var z = 0;
        for (var i = 1; i < n; ++i)
            if (x[i] > y) {
                y = x[i];
                z = i;
            }
        return {y, z};
    }
}

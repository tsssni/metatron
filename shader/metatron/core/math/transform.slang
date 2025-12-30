implementing math;

namespace mtt::math {
    public struct Transform {
        public fm4 transform;
        public fm4 inv_transform;
        public __init() {
            let identity = fm4(
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1,
            );
            transform = inv_transform = identity;
        }
        public __init(fm4 transform) {
            this.transform = transform;
            this.inv_transform = inverse(transform);
        }
        public __init(fm4 transform, fm4 inv_transform) {
            this.transform = transform;
            this.inv_transform = inv_transform;
        }
    }
}

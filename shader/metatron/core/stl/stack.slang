implementing stl;

namespace mtt {
    public struct buf<T> {
        public Ptr<T> ptr = nullptr;
        public u32 bytelen = 0;
        public u32 idx = 0xffffffffu;

        public __subscript(u32 i) -> T {
            get { return ptr[i]; }
        }

        public func size() -> u32 {
            return bytelen / sizeof(T);
        }

        public func empty() -> bool {
            return bytelen == 0;
        }
    }
}

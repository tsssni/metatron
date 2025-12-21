implementing stl;

namespace mtt {
    public struct buf<T> {
        Ptr<T> ptr = nullptr;
        uptr handle = 0;
        u32 bytelen = 0;
        u32 idx = 0xffffffffu;

        public __subscript(u32 i) -> T {
            get { return ptr[i]; }
        }
    }
}

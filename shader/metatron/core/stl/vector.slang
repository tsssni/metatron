implementing stl;

namespace mtt {
    public struct tag<T> {
        public u32 idx;
        public __init() { idx = 0xffffffffu; }
        public __init<U>(tag<U> t) { idx = t.idx; }

        public func storage() -> u32 { return idx >> 24; }
        public func type() -> u32 { return idx >> 20 & 0xf; };
        public func index() -> u32 { return idx & 0xffff; };
        public func empty() -> bool { return idx == 0xffffffffu; }
        public func get() -> Ptr<T> {
            let arr = Ptr<T>(global.vectors[storage()]);
            return arr + index();
        }
    }
}

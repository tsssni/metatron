implementing stl;

namespace mtt::stl {
    public struct Tag {
        u32 idx;

        public func storage() -> u32 { return idx >> 24; }
        public func type() -> u32 { return idx >> 20 & 0xf; };
        public func index() -> u32 { return idx & 0xffff; };
        public func empty() -> bool { return idx == 0xffffffffu; }
    };
}

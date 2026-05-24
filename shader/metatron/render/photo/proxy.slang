implementing photo;

namespace mtt::photo::proxy {
    public struct Film {
        public tag<photo::Film> idx;
        public func get() -> Ptr<photo::Film> { return idx.get(); }
        public Fixel operator()(filter::Filter filter, uv2 pixel, uv2 size, fv2 u) {
            return get()(filter, pixel, size, u);
        }
    }
}

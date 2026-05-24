implementing math;

namespace mtt::math::proxy {
    public typealias Transform = tag<math::Transform>;

    public struct Planar_Distribution {
        public tag<math::Planar_Distribution> idx;
        public func get() -> Ptr<math::Planar_Distribution> { return idx.get(); }
        public func sample(fv2 u) -> fv2 { return get().sample(u); }
        public func pdf(fv2 p) -> f32 { return get().pdf(p); }
    }
}

implementing accel;

namespace mtt::accel {
    public struct HWBVH: Acceleration {
        u32 idx = 0xffffffffu;

        public Optional<Interaction> operator()(math::Ray r, fv3 n) {
            let array = Ptr<uptr>(resources.vectors);
            let vector = Ptr<Divider>(array[idx]);
            var rq: RayQuery;
            rq.TraceRayInline(resources.accel, 0, 0xff, r.get());

            while (rq.Proceed()) {
                if (rq.CandidateType() == CANDIDATE_PROCEDURAL_PRIMITIVE) {
                    let div = vector[rq.CandidateInstanceIndex()];
                    let or = math::Ray(
                        rq.CandidateRayObjectRayOrigin(),
                        rq.CandidateObjectRayDirection()
                    );
                    let intr_opt = div.shape.query(or, rq.CandidatePrimitiveIndex());
                    if (intr_opt != none) rq.CommitProceduralPrimitiveHit(intr_opt.value[3]);
                }
            }
            if (rq.CommittedStatus() == COMMITTED_NOTHING) return none;

            let div = vector[rq.CommittedInstanceIndex()];
            let prim = rq.CommittedPrimitiveIndex();
            let or = math::Ray(
                rq.CommittedRayObjectRayOrigin(),
                rq.CommittedObjectRayDirection()
            );
            let on = *div.local_to_render.get() / n;
            let ht = rq.CommittedRayT();
            let hb = rq.CommittedTriangleBarycentrics();
            let pos = fv4(1 - hb[0] - hb[1], hb[0], hb[1], ht);
            let intr = div.shape(or, on, pos, prim);
            return Interaction(div, prim, intr);
        }
    }
}

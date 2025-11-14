#pragma once

namespace mtt::stl {
    #define MTT_OPT_OR_CALLBACK(x, opt, f)\
        auto x##_opt = opt;\
        if (!x##_opt) f\
        auto& x = x##_opt.value()

    #define MTT_OPT_OR_BREAK(x, opt) MTT_OPT_OR_CALLBACK(x, opt, {break;})
    #define MTT_OPT_OR_CONTINUE(x, opt) MTT_OPT_OR_CALLBACK(x, opt, {continue;})
    #define MTT_OPT_OR_RETURN(x, opt, ...) MTT_OPT_OR_CALLBACK(x, opt, {return __VA_ARGS__;})
}

namespace mtt {
    template<typename T>
    using opt = std::optional<T>;
}

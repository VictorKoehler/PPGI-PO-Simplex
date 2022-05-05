#pragma once

#include <Eigen/Dense>

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

#define FOR_TO(var, limit) for (decltype((limit)) var = 0; var < limit; ++var)

#ifdef NDEBUG
static const bool ON_DEBUG = false;
#else
static const bool ON_DEBUG = true;
#endif
extern char const *const GIT_COMMIT;

namespace Xplex {
    using Eigen::MatrixXd;
    using Eigen::VectorXd;

    typedef int OptIndex;
}
#pragma once

#include <Eigen/Dense>

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

#define FOR_TO(var, limit) for (decltype((limit)) var = 0; var < limit; ++var)

extern char const *const GIT_COMMIT;
extern int DEFAULT_VERBOSITY;

#ifdef NDEBUG
static const bool ON_DEBUG = false;
#else
static const bool ON_DEBUG = true;
#endif

namespace Xplex {
    using Eigen::MatrixXd;
    using Eigen::VectorXd;

    typedef int OptIndex;
}
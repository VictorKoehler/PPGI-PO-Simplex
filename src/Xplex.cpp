#include <iostream>
#include "global.hpp"
#include "Xplex.hpp"

namespace Xplex {
    void Xplex::solve() {
        const auto m = size_t(model->b.rows());
        const auto nn = model->variables.size() - m;
        A_B_m1 = MatrixXd::Identity(m, m); // inverse of identity is itself
        xc_b = model->b;
        basic_vars.reserve(m);
        variable_is_basic.resize(nn, false);
        for (size_t i = 0; i < m; i++) {
            basic_vars.push_back(nn + i);
            variable_is_basic.push_back(true);
        }

        revised_simplex();
    }

    template<typename T>
    inline auto eigen_argmax(T &z) {
        decltype(z.size()) argmax = 0;
        for (decltype(z.size()) i = 0; i < z.size(); i++) {
            if (z(i) > z(argmax)) argmax = i;
        }
        return argmax;
    }

    template<typename T>
    inline auto eigen_argmin(T &z) {
        decltype(z.size()) argmin = 0;
        for (decltype(z.size()) i = 0; i < z.size(); i++) {
            if (z(i) < z(argmin)) argmin = i;
        }
        return argmin;
    }

    void Xplex::revised_simplex() {
        while (true) {
            std::vector<uint> non_basic_vars; // TODO: Find a better solution
            non_basic_vars.reserve(getNonBasisSize());
            for (size_t i = 0; i < variable_is_basic.size(); i++) {
                if (!variable_is_basic[i]) non_basic_vars.push_back(i);
            } // non_basic_vars := j=non_basic_vars[i] -> v=model->variables[j] such that v is non basic
            assert(non_basic_vars.size() == getNonBasisSize());

            if (unlikely(isVerbose())) {
                std::cout << "Basic Variables: ";
                for (const auto i : basic_vars) std::cout << " " << model->variables[i].getName() << "(" << i << ":" << model->c[i] << ")";
                std::cout << "\n";
                std::cout << "Non-Basic Variables:";
                for (const auto i : non_basic_vars) std::cout << " " << model->variables[i].getName() << "(" << i << ":" << model->c[i] << ")";
                std::cout << "\n";
            }

            // std::cout << "[" << model->c(non_basic_vars).transpose() << "] - [" << model->c(basic_vars).transpose() << "] [\n" << A_B_m1 << "] [\n" << model->A(Eigen::all, non_basic_vars) << "]\n\n";
            // std::cout << "[" << model->c(non_basic_vars).transpose() << "] - [" << (model->c(basic_vars).transpose() * A_B_m1) << "] [\n" << model->A(Eigen::all, non_basic_vars) << "]\n\n";
            // std::cout << "[" << model->c(non_basic_vars).transpose() << "] - [" << (model->c(basic_vars).transpose() * A_B_m1 * model->A(Eigen::all, non_basic_vars)) << "]\n\n";
            auto coeffz = model->c(non_basic_vars).transpose() - model->c(basic_vars).transpose() * A_B_m1 * model->A(Eigen::all, non_basic_vars);
            // std::cout << coeffz << "\n\n";
            auto coeffz_argmax = eigen_argmax(coeffz);
            const auto entering_var = non_basic_vars[coeffz_argmax];
            if (unlikely(isVerbose())) std::cout << "Z coeff NB: " << coeffz << ": " << coeffz_argmax << "\n";
            if (coeffz[coeffz_argmax] <= 0) {
                // TODO: Finish routine
                break;
            }

            auto d = A_B_m1 * model->A(Eigen::all, entering_var);
            auto t = (xc_b.array() / d.array()).eval();
            auto t_argmin = eigen_argmin(t);
            if (unlikely(isVerbose())) {
                std::cout << "Exiting variable: " << model->variables[basic_vars[t_argmin]].getName() << " (" << basic_vars[t_argmin]
                        << "), entering: " << model->variables[entering_var].getName() << " (" << entering_var << ")\n";
                std::cout << "xc_b   = [" << xc_b.transpose() << "].T;\nxc_b' -= ";
                std::cout << "(d=[" << d.transpose() << "]).T * (t=[" << t.transpose() << "]).T: " << t_argmin << "\n";
            }
            if (t[t_argmin] < 0) {
                // TODO: Unlimited
                break;
            }

            xc_b -= t[t_argmin] * d; // Changes class state
            xc_b(t_argmin) = t[t_argmin]; // Changes class state
            if (unlikely(isVerbose())) std::cout << "xc_b'  = [" << xc_b.transpose() << "].T\n";

            MatrixXd E = MatrixXd::Identity(getBasisSize(), getBasisSize());
            E(Eigen::all, t_argmin) = -d / d(t_argmin);
            E(t_argmin, t_argmin) = 1/d(t_argmin);

            A_B_m1 = (E * A_B_m1).eval(); // Changes class state
            variable_is_basic[basic_vars[t_argmin]] = false; // Changes class state
            variable_is_basic[entering_var] = true; // Changes class state
            basic_vars[t_argmin] = entering_var; // Changes class state
            iterations++;

            assert((model->A(Eigen::all, basic_vars).inverse() - A_B_m1).cwiseAbs().maxCoeff() < 0.1
                    && "The matrix inversion method produced a different result than Eigen's inverse");

            if (unlikely(isVerbose())) std::cout << "E:\n" << E << "\nA_B-1:\n" << A_B_m1 << "\n";
        }
    }
}
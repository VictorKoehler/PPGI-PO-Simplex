#include <iostream>
#include <unordered_set>
#include "global.hpp"
#include "Xplex.hpp"

namespace Xplex {
    static const double EPSILON = 1e-5;
    Xplex::Xplex(const Model *model) : model(model), iterations(0), check_cycles(0), verbose(false), phase1(false) { }

    Xplex::Xplex(Model *model) : Xplex(const_cast<const Model *>(model)) {
        if (!model->isBuilt()) model->build();
    }

    void Xplex::solve() {
        if (!model->isBuilt()) throw std::runtime_error("You must build the model before solving.");
        // if (unlikely(isVerbose())) model->print();

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

        phase1 = model->isTwoPhaseNeeded();
        if (phase1) {
            std::cout << "Phase I started\n";
            revised_simplex();
            phase1 = false;
            std::cout << "Phase I finished\n";
        }
        std::cout << "Phase II started\n";
        revised_simplex();
        std::cout << "Phase II finished\n";
    }

    template<typename T>
    inline auto eigen_arg_first_positive(T &z) {
        FOR_TO(i, z.size()) {
            if (z(i) > EPSILON) return i;
        }
        decltype((z.size())) dummy = 0;
        return dummy;
    }

    template<typename T>
    inline auto eigen_argmax(T &z) {
        decltype(z.size()) argmax = 0;
        FOR_TO(i, z.size()) {
            if (z(i) > z(argmax)) argmax = i;
        }
        return argmax;
    }

    template<typename T>
    inline auto eigen_argmin(T &z) {
        decltype(z.size()) argmin = 0;
        FOR_TO(i, z.size()) {
            if (z(i) < z(argmin)) argmin = i;
        }
        return argmin;
    }

    #define model_c(...) (phase1 ? model->c_art(__VA_ARGS__) : model->c(__VA_ARGS__))

    template<typename T>
    std::string e_print(T v, size_t space=5) {
        std::string b = "[";
        b.reserve(v.size()*(space+1)+4);
        for (const auto& i : v) {
            const auto ir = std::to_string(i).substr(0, space);
            b.append(ir + std::string(space - ir.size() + 1, ' '));
        }
        b.pop_back();
        b.push_back(']');
        return b;
    }

    std::size_t calculate_hash(std::vector<uint> const& vec) {
        std::size_t seed = vec.size();
        for(auto x : vec) {
            x = ((x >> 16) ^ x) * 0x45d9f3b;
            x = ((x >> 16) ^ x) * 0x45d9f3b;
            x = (x >> 16) ^ x;
            seed ^= x + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }

    void Xplex::revised_simplex() {
        if (unlikely(isVerbose())) {
            if (phase1) std::cout << "ç: " << model->c_art.transpose() << "\n";
            else std::cout << "c: " << model->c.transpose() << "\n";
            std::cout << "b: " << model->b.transpose() << "\n";
            std::cout << "A: [\n" << model->A << " ]\n\n";
            std::cout << "A_B-1: [\n" << A_B_m1 << " ]\n\n\n";
        }

        const auto it_bef = iterations;
        std::unordered_map<size_t, std::unordered_set<double>> past_basis;
        uint cycles = check_cycles;

        while (true) {
            iterations++;
            std::vector<uint> non_basic_vars; // TODO: Find a better solution
            non_basic_vars.reserve(getNonBasisSize());
            FOR_TO(i, variable_is_basic.size()) {
                if (!variable_is_basic[i] && (phase1 || model->variables[i].getType() != Variable::Type::Artificial))
                    non_basic_vars.push_back(i);
            } // non_basic_vars := j=non_basic_vars[i] -> v=model->variables[j] such that v is non basic

            if (unlikely(isVerbose())) {
                std::cout << "=== ITERATION #" << iterations << " (PHASE I" << (phase1 ? "" : "I") << ") ===\n";
                std::cout << "Current Z: " << std::to_string(getObjValue()) << "\n";
                std::cout << "Basic Variables: ";
                for (const auto i : basic_vars) std::cout << " " << model->variables[i].getName() << "(" << i << ":" << model_c(i) << ")";
                std::cout << "\n";
                std::cout << "Non-Basic Variables:";
                for (const auto i : non_basic_vars) std::cout << " " << model->variables[i].getName() << "(" << i << ":" << model_c(i) << ")";
                std::cout << "\n\n";

                std::cout << "c_N: " << model_c(non_basic_vars).transpose() << "\n";
                std::cout << "c_B: " << model_c(basic_vars).transpose() << "\n";
                std::cout << "A_N (aka N):\n" << model->A(Eigen::all, non_basic_vars) << "\n";
            }

            if (unlikely(isCheckingCycles())) {
                const auto c_hash = calculate_hash(basic_vars);
                const auto c_obj = xc_b.sum();
                auto b_pb = past_basis.find(c_hash);
                if (b_pb == past_basis.end()) { // absent
                    past_basis[c_hash] = {c_obj};
                    if (cycles < check_cycles) cycles++;
                } else if (b_pb->second.find(c_obj) == b_pb->second.end()) { // absent
                    b_pb->second.insert(c_obj);
                    if (cycles < check_cycles) cycles++;
                } else { // Cycling detected
                    cycles--;
                    if (cycles == 0) throw std::runtime_error("Possible cycling detected! Aborting...");
                }
            }

            // std::cout << "[" << model_c(non_basic_vars).transpose() << "] - [" << model_c(basic_vars).transpose() << "] [\n" << A_B_m1 << "] [\n" << model->A(Eigen::all, non_basic_vars) << "]\n\n";
            // std::cout << "[" << model_c(non_basic_vars).transpose() << "] - [" << (model_c(basic_vars).transpose() * A_B_m1) << "] [\n" << model->A(Eigen::all, non_basic_vars) << "]\n\n";
            // std::cout << "[" << model_c(non_basic_vars).transpose() << "] - [" << (model_c(basic_vars).transpose() * A_B_m1 * model->A(Eigen::all, non_basic_vars)) << "]\n\n";

            auto coeffz = model_c(non_basic_vars).transpose() - model_c(basic_vars).transpose() * A_B_m1 * model->A(Eigen::all, non_basic_vars);

            // auto coeffz_argmax = eigen_argmax(coeffz); // Usual rule
            auto coeffz_argmax = eigen_arg_first_positive(coeffz); // Bland's rule

            const auto entering_var = non_basic_vars[coeffz_argmax];
            if (unlikely(isVerbose())) {
                std::cout << "Z coeff NB: " << e_print(coeffz) << ":=> c[cin=" << coeffz_argmax << "]=" << coeffz[coeffz_argmax] << "\n";
            }
            if (coeffz[coeffz_argmax] <= 0) {
                // TODO: Finish routine
                std::cout << "No coefficient improves the current solution. Exiting...\n";
                break;
            }

            auto d = A_B_m1 * model->A(Eigen::all, entering_var);
            auto t = (xc_b.array() / d.array()).eval();
            int t_argmin = -1, d_argpos = -1;
            FOR_TO(i, t.size()) {
                if (d(i) > EPSILON) d_argpos = i;
                else if (d(i) <= 0) continue;
                if (t(i) >= 0 && (t_argmin == -1 || t(i) < t(t_argmin)) && !std::isinf(t(i))) {
                    t_argmin = int(i);
                }
            }
            

            if (unlikely(isVerbose())) {
                std::cout << "A[:,e]= " << e_print(model->A(Eigen::all, entering_var)) << ".T;\n";
                std::cout << "xc_b  = " << e_print(xc_b) << ".T;\n";
                std::cout << "d     = " << e_print(d) << ".T = A_B-1 * A[:,entering]; d[" << d_argpos << "]>0\n";
                std::cout << "t     = " << e_print(t) << ".T = xc_b/d; => t[t_min=" << t_argmin << "]=";
                if (t_argmin >= 0) {
                    std::cout << t[t_argmin] << (t[t_argmin] > 0 ? " >" : " =") << " 0\nExiting variable: " <<
                                 model->variables[basic_vars[t_argmin]].getName() << " (" << basic_vars[t_argmin] <<
                                 "), entering: " << model->variables[entering_var].getName() << " (" << entering_var << ")\n";
                } else {
                    std::cout << "*\n";
                }
            }
            if (t_argmin == -1 || d_argpos == -1) {
                // TODO: Unlimited
                std::cout << "Problem is unlimited\n";
                if (unlikely(isVerbose())) std::cout << "\nA_B-1:\n" << A_B_m1 << "\n\n\n";
                return;
            }

            xc_b -= t[t_argmin] * d; // Changes class state
            xc_b(t_argmin) = t[t_argmin]; // Changes class state
            if (unlikely(isVerbose())) {
                std::cout << "xc_b' = [" << xc_b.transpose() << "].T = xc_b - t[t_min] * d; xc_b'[t_min]=t[t_min]\n\n";
            }

            MatrixXd E = MatrixXd::Identity(getBasisSize(), getBasisSize());
            E(Eigen::all, t_argmin) = -d / d(t_argmin);
            E(t_argmin, t_argmin) = 1/d(t_argmin);

            A_B_m1 = (E * A_B_m1).eval(); // Changes class state
            variable_is_basic[basic_vars[t_argmin]] = false; // Changes class state
            variable_is_basic[entering_var] = true; // Changes class state
            basic_vars[t_argmin] = entering_var; // Changes class state

            if constexpr (ON_DEBUG) {
                const auto a_real_inversed = model->A(Eigen::all, basic_vars).inverse().eval();
                const auto c = (a_real_inversed - A_B_m1).cwiseAbs().maxCoeff() < 0.001;
                if (!c) {
                    std::cerr << "A(:,basic_vars={";
                    for (const auto i : basic_vars) std::cerr << " " << model->variables[i].getName();
                    std::cerr << " }):\n" << model->A(Eigen::all, basic_vars) << "\nEIGEN (REAL) INVERSION:\n"
                        << a_real_inversed << "\n TRICK (EXPECTED) INVERSION:\n" << A_B_m1 << "\nFROM E:\n" << E << "\n";
                }
                assert(c && "The matrix inversion method produced a different result than Eigen's inverse");

                const auto bv = (model->A(Eigen::all, basic_vars) * xc_b - model->b).cwiseAbs().maxCoeff() < 0.001;
                if (!bv) {
                    std::cerr << "Computed: " << e_print(model->A(Eigen::all, basic_vars) * xc_b, 3) << "\n";
                    std::cerr << "b limit:  " << e_print(model->b, 3) << "\n";
                }
                assert(bv && "This solution violated constraints!");
            }

            if (unlikely(isVerbose())) std::cout << "E:\n" << E << "\nA_B-1:\n" << A_B_m1 << "\n\n\n";
        }

        if (!phase1) {
            if (unlikely(isVerbose())) std::cout << "\nA_B-1:\n" << A_B_m1 << "\n\n";
            std::cout << "Solution found: z = " << getObjValue() << "\n";
            std::cout << "After " << iterations << " iterations";
            if (it_bef == 0) std::cout << "\n";
            else std::cout << ", of which " << it_bef << " were on Phase I\n";

            std::cout << "Basic variables:";
            FOR_TO(i, basic_vars.size()) {
                std::cout << "  " << model->variables[basic_vars[i]].getName() << "=" << xc_b(i);
            }
            std::cout << "\nNon-Basic variables: ";
            FOR_TO(i, variable_is_basic.size()) {
                if (variable_is_basic[i]) continue;
                std::cout << model->variables[i].getName() << " = ";
            }

            const auto y = model_c(basic_vars).transpose() * A_B_m1;
            std::cout << "0\nDual variables: y* = [" << y << "]\n\n";

            std::cout << "Range Sensibility:\n";
            FOR_TO(i, xc_b.size()) {
                const auto delta = (-xc_b.array() / A_B_m1(Eigen::all, i).array()).eval();
                double closest_neg = -INFINITY, closest_pos = +INFINITY;
                for (const double v : delta) {
                    if (v < 0) {
                        if (v > closest_neg) closest_neg = v;
                    } else {
                        if (v < closest_pos) closest_pos = v;
                    }
                }

                const auto b = model->b(i);
                bool tail=false;
                std::cout << "  ";
                for (const auto& vb_ind : basic_vars) {
                    if (tail) std::cout << "  +  ";
                    else tail = true;
                    std::cout << model->A(i, vb_ind) << " " << model->variables[vb_ind].getName();
                }
                std::cout << " = " << xc_b.dot(model->A(i, basic_vars)) << " <=  b=" << b << ";   Range: ";
                std::cout << b+closest_neg << " <= b <= " << b+closest_pos << "\n";
            }
        }
    }

    double Xplex::getObjValue() const {
        // return model->objc.getScalar() + model_c(basic_vars).transpose() * A_B_m1 * model->b;
        const double r = model_c(basic_vars).transpose() * xc_b;
        return model->objc.getOriginalObjectiveType() == model->objc.getObjectiveType() ? r : -r;
    }

    double Xplex::getValue(const Variable& v) const {
        if (!variable_is_basic[v.getIndex()]) return 0;
        FOR_TO(i, basic_vars.size()) {
            if (int(basic_vars[i]) == v.getIndex()) {
                return xc_b(i);
            }
        }
        throw std::runtime_error("Unexpected error: Variable is basic but was not found.");
    }
}

#include <iostream>
#include <unordered_set>
#include "global.hpp"
#include "Xplex.hpp"

namespace Xplex {
    static const double EPSILON = 1e-5;
    Xplex::Xplex(const Model *model) : model(model), iterations(0), check_cycles(0), timelimit(0), verbose(false), phase1(false) { }

    Xplex::Xplex(Model *model) : Xplex(const_cast<const Model *>(model)) {
        if (!model->isBuilt()) model->build();
    }

    void Xplex::solve() {
        if (!model->isBuilt()) throw std::runtime_error("You must build the model before solving.");
        // if (isVeryVerbose()) model->print();

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
        timeStarted = TimePoint();
        bool cont = true;
        if (phase1) {
            std::cout << "Phase I started at " << timeStarted << "\n";
            cont = revised_simplex();
            phase1 = false;
            TimePoint tpp1;
            std::cout << "Phase I finished at " << tpp1 << "; Total clock time elapsed: "
                      << timeStarted.diff_seconds(tpp1) << " seconds" << std::endl;
        }
        TimePoint tpp2;
        if (cont) {
            TimePoint tpp2s;
            std::cout << "Phase II started at " << tpp2s << "\n";
            revised_simplex();
            tpp2 = TimePoint();
            std::cout << "Phase II finished at " << tpp2 << "; Total clock time elapsed: "
                        << tpp2s.diff_seconds(tpp2) << " seconds" << std::endl;
        }

        if (model->isTwoPhaseNeeded()) {
            std::cout << "Total clock time elapsed on both phases: " << timeStarted.diff_seconds(tpp2) << std::endl;
        }
    }

    template<typename T, typename TR=double>
    inline TR eigen_arg_first_positive(const T &z) {
        FOR_TO(i, z.size()) {
            if (z(i) > EPSILON) return i;
        }
        decltype((z.size())) dummy = 0;
        return dummy;
    }

    template<typename T, typename TR=double>
    inline TR eigen_argmax(const T &z) {
        decltype(z.size()) argmax = 0;
        FOR_TO(i, z.size()) {
            if (z(i) > z(argmax)) argmax = i;
        }
        return argmax;
    }

    template<typename T, typename TR=double>
    inline TR eigen_argmin(const T &z) {
        decltype(z.size()) argmin = 0;
        FOR_TO(i, z.size()) {
            if (z(i) < z(argmin)) argmin = i;
        }
        return argmin;
    }

    #define model_c(...) (phase1 ? model->c_art(__VA_ARGS__) : model->c(__VA_ARGS__))
    #define isVeryVerbose() unlikely(getVerbosityLevel() >= 4)

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

    bool Xplex::revised_simplex() {
        if (isVeryVerbose()) {
            if (phase1) std::cout << "ç: " << model->c_art.transpose() << "\n";
            else std::cout << "c: " << model->c.transpose() << "\n";
            std::cout << "b: " << model->b.transpose() << "\n";
            std::cout << "A: [\n" << model->A << " ]\n\n";
            std::cout << "A_B-1: [\n" << A_B_m1 << " ]\n\n\n";
        }

        const auto it_bef = iterations;
        std::unordered_map<size_t, std::unordered_set<double>> past_basis;
        const uint max_cycles = check_cycles;

        std::vector<uint> non_basic_vars; // TODO: Find a better solution
        while (true) {
            iterations++;
            non_basic_vars.clear();
            non_basic_vars.reserve(getNonBasisSize());
            FOR_TO(i, variable_is_basic.size()) {
                if (!variable_is_basic[i] && (phase1 || model->variables[i].getType() != Variable::Type::Artificial))
                    non_basic_vars.push_back(i);
            } // non_basic_vars := j=non_basic_vars[i] -> v=model->variables[j] such that v is non basic

            if (unlikely(iterations % 1000 == 0)) {
                auto et = timeStarted.diff_seconds();
                if (isVerbose()) {
                    std::cout << "=== ITERATION #" << iterations << " (PHASE I" << (phase1 ? ", " : "I, ") << et
                              << " seconds elapsed); Current Z = " << std::to_string(getObjValue()) << " ===" << std::endl;
                }
                if (isTimeLimited() && et > double(timelimit)) {
                    std::cout << "TIME LIMIT REACHED!" << std::endl;
                    if (isVerbose()) print_statedbg(non_basic_vars, true);
                    return false;
                }
            } else if (isVeryVerbose()) {
                print_statedbg(non_basic_vars);
            }

            if (isCheckingCycles()) {
                const auto c_hash = calculate_hash(basic_vars);
                const auto c_obj = xc_b.sum();
                auto b_pb = past_basis.find(c_hash);
                if (b_pb == past_basis.end()) { // absent
                    past_basis[c_hash] = {c_obj};
                    if (check_cycles < max_cycles) check_cycles++;
                } else if (b_pb->second.find(c_obj) == b_pb->second.end()) { // absent
                    b_pb->second.insert(c_obj);
                    if (check_cycles < max_cycles) check_cycles++;
                } else { // Cycling detected
                    check_cycles--;
                    if (check_cycles == 0) {
                        if (isVerbose()) std::cout << "CYCLING!\n";
                        if (isVerbose()) print_statedbg(non_basic_vars, true);
                        throw std::runtime_error("Possible cycling detected! Aborting...");
                    }
                }
            }

            // std::cout << "[" << model_c(non_basic_vars).transpose() << "] - [" << model_c(basic_vars).transpose() << "] [\n" << A_B_m1 << "] [\n" << model->A(Eigen::all, non_basic_vars) << "]\n\n";
            // std::cout << "[" << model_c(non_basic_vars).transpose() << "] - [" << (model_c(basic_vars).transpose() * A_B_m1) << "] [\n" << model->A(Eigen::all, non_basic_vars) << "]\n\n";
            // std::cout << "[" << model_c(non_basic_vars).transpose() << "] - [" << (model_c(basic_vars).transpose() * A_B_m1 * model->A(Eigen::all, non_basic_vars)) << "]\n\n";

            Eigen::RowVectorXd coeffz = model_c(non_basic_vars).transpose() - model_c(basic_vars).transpose() * A_B_m1 * model->A(Eigen::all, non_basic_vars);

            // auto coeffz_argmax = eigen_argmax(coeffz); // Usual rule
            auto coeffz_argmax = eigen_arg_first_positive(coeffz); // Bland's rule

            const auto entering_var = non_basic_vars[coeffz_argmax];
            if (isVeryVerbose()) {
                std::cout << "Z coeff NB: " << e_print(coeffz) << ":=> c[cin=" << coeffz_argmax << "]=" << coeffz[coeffz_argmax] << "\n";
            }
            if (coeffz[coeffz_argmax] <= 0) {
                // TODO: Finish routine
                std::cout << "No coefficient improves the current solution. Exiting...\n";
                if (getVerbosityLevel() >= 2) print_statedbg(non_basic_vars, true);
                break;
            }

            Eigen::VectorXd d = A_B_m1 * model->A(Eigen::all, entering_var);
            Eigen::ArrayXd t = (xc_b.array() / d.array()).eval();
            int t_argmin = -1, d_argpos = -1;
            FOR_TO(i, t.size()) {
                if (d(i) > EPSILON) d_argpos = i;
                else if (d(i) <= 0) continue;
                if (t(i) >= 0 && (t_argmin == -1 || t(i) < t(t_argmin)) && !std::isinf(t(i))) {
                    t_argmin = int(i);
                }
            }
            

            if (isVeryVerbose()) {
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
                if (isVerbose()) print_statedbg(non_basic_vars, true);
                if (isVeryVerbose()) std::cout << "\nA_B-1:\n" << A_B_m1 << "\n\n\n";
                return false;
            }

            xc_b -= t[t_argmin] * d; // Changes class state
            xc_b(t_argmin) = t[t_argmin]; // Changes class state
            if (isVeryVerbose()) {
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
                const auto c = (a_real_inversed - A_B_m1).cwiseAbs().maxCoeff() < 0.01;
                if (!c) {
                    print_statedbg(non_basic_vars, true);
                    std::cerr << "A(:,basic_vars={";
                    for (const auto i : basic_vars) std::cerr << " " << model->variables[i].getName();
                    std::cerr << " }):\n" << model->A(Eigen::all, basic_vars) << "\nEIGEN (REAL) INVERSION:\n"
                        << a_real_inversed << "\n TRICK (EXPECTED) INVERSION:\n" << A_B_m1 << "\nFROM E:\n" << E << "\n";
                }
                assert(c && "The matrix inversion method produced a different result than Eigen's inverse");

                const auto bv = (model->A(Eigen::all, basic_vars) * xc_b - model->b).cwiseAbs().maxCoeff() < 0.001;
                if (!bv) {
                    print_statedbg(non_basic_vars, true);
                    std::cerr << "Computed: " << e_print(model->A(Eigen::all, basic_vars) * xc_b, 3) << "\n";
                    std::cerr << "b limit:  " << e_print(model->b, 3) << "\n";
                }
                assert(bv && "This solution violated constraints!");
            }

            if (isVeryVerbose()) std::cout << "E:\n" << E << "\nA_B-1:\n" << A_B_m1 << "\n\n\n";
        }

        if (!phase1) {
            if (isVeryVerbose()) print_statedbg(non_basic_vars);;
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

            const Eigen::RowVectorXd y = model_c(basic_vars).transpose() * A_B_m1;
            std::cout << "0\nDual variables: y* = [" << y << "]\n\n";

            std::cout << "Range Sensibility:\n";
            FOR_TO(i, xc_b.size()) {
                const Eigen::ArrayXd delta = (-xc_b.array() / A_B_m1(Eigen::all, i).array()).eval();
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
        return true;
    }



    void Xplex::print_statedbg(const std::vector<uint>& non_basic_vars, bool flushout) const {
        auto et = timeStarted.diff_seconds();
        std::cout << "=== ITERATION #" << iterations << " (PHASE I" << (phase1 ? ", " : "I, ") << et << " seconds elapsed) ===\n";
        std::cout << "Current Z: " << std::to_string(getObjValue()) << "\n";
        if (isCheckingCycles()) std::cout << "Cycling Tolerance Counter: " << check_cycles << "\n";
        std::cout << "Basic Variables: ";
        for (const auto i : basic_vars) std::cout << " " << model->variables[i].getName() << "(" << i << ":" << model_c(i) << ")";
        std::cout << "\n";
        std::cout << "Non-Basic Variables:";
        for (const auto i : non_basic_vars) std::cout << " " << model->variables[i].getName() << "(" << i << ":" << model_c(i) << ")";
        std::cout << "\n\n";

        std::cout << "c_N: " << model_c(non_basic_vars).transpose() << "\n";
        std::cout << "c_B: " << model_c(basic_vars).transpose() << "\n";
        std::cout << "A_N (aka N):\n" << model->A(Eigen::all, non_basic_vars) << "\n";

        if (flushout) std::cout << std::endl;
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

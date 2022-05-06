#include <iostream>
#include "global.hpp"
#include "Model.hpp"

namespace Xplex {
    void Model::build() {
        std::cout << "Building model...\n";
        clear_artificials();

        for (const auto& v : variables) {
            if (v.getDomain() == Variable::Domain::Unbounded) {
                const auto vi = OptIndex(variables.size());
                variables.push_back(Variable("co_"+v.getName(), vi, Variable::Type::NegativeMirror, Variable::Domain::NonNegative));

                objc.setVariableCoefficient(variables[vi], -objc.getVariableCoefficient(v));
                for (auto& c : constraints) {
                    c.setVariableCoefficient(variables[vi], -c.getVariableCoefficient(v));
                }
            }
        }

        for (auto& c : constraints) {
            if (c.getScalar() < 0) c.multiplyBy(-1);
            if (c.getInequalityType() == Constraint::InequalityType::GreaterOrEqual) {
                const auto vi = OptIndex(variables.size());
                const auto n = "v"+std::to_string(vi)+"_cp"+std::to_string(c.getIndex());
                variables.push_back(Variable(n, vi, Variable::Type::NegativeSlack, Variable::Domain::NonNegative));
                c.setVariableCoefficient(variables[vi], -1.0);
            }
        }
        for (auto& c : constraints) {
            const bool slack = c.getInequalityType() == Constraint::InequalityType::LessOrEqual; // !slack => artificial
            const auto vi = OptIndex(variables.size());
            const auto n = "v"+std::to_string(vi)+(slack ? "_cs" : "_ca")+std::to_string(c.getIndex());
            variables.push_back(Variable(n, vi, (slack ? Variable::Type::Slack : Variable::Type::Artificial), Variable::Domain::NonNegative));
            c.setVariableCoefficient(variables[vi], 1.0);
            
            if (!slack) artificialObjective.setVariableCoefficient(variables[vi], -1.0);
        }
        if (objective().getObjectiveType() == ObjectiveFunction::Minimization) {
            objective().multiplyBy(-1); // We only Maximize!
        }


        c_art = VectorXd::Zero(variables.size());
        c = VectorXd::Zero(variables.size());
        b = VectorXd::Zero(constraints.size());
        A = MatrixXd::Zero(constraints.size(), variables.size());

        for (const auto& ci : artificialObjective.comp_variables) {
            c_art(ci.first) = (variables[ci.first].getDomain() == Variable::Domain::NonPositive ? -1 : 1) * ci.second;
        }
        for (const auto& ci : objective().comp_variables) {
            c(ci.first) = (variables[ci.first].getDomain() == Variable::Domain::NonPositive ? -1 : 1) * ci.second;
        }

        for (const auto& c : constraints) {
            b(c.getIndex()) = c.getScalar();
            for (const auto& cv : c.comp_variables) {
                A(c.getIndex(), cv.first) = (variables[cv.first].getDomain() == Variable::Domain::NonPositive ? -1 : 1) * cv.second;
            }
        }

        built = true;
    }

    void Model::clear_artificials() {
        if (built) {
            built = false;
            auto nsize = variables.size();
            while (nsize > 0 && variables[nsize-1].getType() != Variable::Type::User) {
                nsize--;
            }

            artificialObjective.comp_variables.clear();
            for (auto& c : constraints) {
                for (auto va_i = nsize; va_i < variables.size(); va_i++) {
                    assert(variables[va_i].getType() != Variable::Type::User);
                    c.comp_variables.erase(variables[va_i].getIndex());
                }
            }

            assert(nsize < variables.size());
            variables.resize(nsize, Variable(""));
            if constexpr (ON_DEBUG) {
                for (const auto& v : variables) {
                    assert(v.getType() == Variable::Type::User);
                }
            }
        }
    }
    
    bool Model::isTwoPhaseNeeded() const {
        return !artificialObjective.comp_variables.empty();
    }
    
    bool Model::isBuilt() const {
        return built;
    }

    Model& Model::add(Variable& v) {
        clear_artificials();
        v.index = OptIndex(variables.size());
        variables.push_back(v);
        return *this;
    }

    Model& Model::add(Constraint& c) {
        c.index = OptIndex(constraints.size());;
        return add_discard(c);
    }

    Model& Model::add_discard(const Constraint& c) {
        clear_artificials();
        const auto index = OptIndex(constraints.size());;
        constraints.push_back(c);
        constraints[index].index = index;
        #ifndef NDEBUG
        // constraints[index].dirty = false; // TODO: Complete
        #endif
        return *this;
    }

    Variable Model::newVariable(const std::string& name, Variable::Domain domain) {
        clear_artificials();
        auto i = OptIndex(variables.size());
        auto v = Variable(name.empty() ? "v"+std::to_string(i) : name, domain);
        v.index = i;
        variables.push_back(v);
        return v;
    }

    // Constraint&& Model::newConstraint(const std::string& name, double b_i, Constraint::InequalityType restr) {
    //     return add(Constraint(name, b_i, restr));
    // }

    inline std::string double_tostr(double d) {
        const auto ss = std::to_string(d);
        return ss.substr(0, ss.find(".") + 2);
    }

    inline auto print_expr(const std::vector<Xplex::Variable>& variables, const Expression* e, std::vector<size_t>& maxs) {
        std::vector<std::string> r;
        for (const auto& v : variables) {
            const auto s = e->getVariableCoefficient(v);
            const auto rs = s == 0 ? "" : double_tostr(s) + " " + v.getName();
            r.push_back(rs);
            if (rs.size() > maxs[v.getIndex()]) maxs[v.getIndex()] = rs.size();
        }
        return r;
    }

    void Model::print() const {
        std::vector<std::vector<std::string>> cols;
        std::vector<size_t> maxs(variables.size(), 0);

        cols.reserve(constraints.size()+1);
        cols.push_back(print_expr(variables, &objc, maxs));
        std::cout << (objc.getObjectiveType() == ObjectiveFunction::ObjectiveType::Maximization ? "Maximize:\n" : "Minimize:\n");

        for (const auto& c : constraints) {
            cols.push_back(print_expr(variables, &c, maxs));
        }

        FOR_TO (c_, cols.size()) {
            const auto& c = cols[c_];
            FOR_TO (i, c.size()) {
                const auto& s = c[i];
                bool neg = s.size() >= 1 && s[0] == '-';
                const auto s_ = neg ? s.substr(1) : s;
                if (i != 0) {
                    std::cout << (s.size() == 0 ? "     " : (neg ? "  -  " : "  +  "));
                }
                std::cout << std::string(maxs[i]-s_.size(), ' ') << s_;
            }

            if (c_ != 0) {
                switch (constraints[c_-1].getInequalityType()) {
                case Constraint::InequalityType::Equal:
                    std::cout << " == ";
                    break;
                case Constraint::InequalityType::LessOrEqual:
                    std::cout << " <= ";
                    break;
                case Constraint::InequalityType::GreaterOrEqual:
                    std::cout << " >= ";
                    break;
                }
                std::cout << constraints[c_-1].getScalar() << "\n";
            } else {
                if (objc.getScalar() != 0) std::cout << (objc.getScalar() < 0 ? " - " : " + ") << abs(objc.getScalar());
                std::cout << "\n\n";
            }
        }

        for (const auto& v : variables) {
            std::cout << (v.getIndex() == 0 ? "\n" : ", ") << v.getName();
        }
        std::cout << " >= 0\n\n";
    }
}

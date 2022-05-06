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
                variables.push_back(Variable("vco_"+v.getName(), vi, Variable::Type::NegativeMirror, Variable::Domain::NonNegative));

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
        if (c.name.empty())
            c.name = "c_" + std::to_string(c.index);
        return add_discard(c);
    }

    Model& Model::add_discard(const Constraint& c) {
        clear_artificials();
        const auto index = OptIndex(constraints.size());;
        constraints.push_back(c);
        constraints[index].index = index;
        if (constraints[index].name.empty())
            constraints[index].name = "c_" + std::to_string(index);
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

    inline auto print_expr(const std::vector<Xplex::Variable>& variables, const Expression* e, std::vector<size_t>& maxs, bool built) {
        std::vector<std::string> r;
        for (const auto& v : variables) {
            const auto neg = built && v.getDomain() == Variable::Domain::NonPositive;
            const auto s = e->getVariableCoefficient(v) * (neg ? -1 : 1);
            const auto rs = s == 0 ? "" : double_tostr(s) + " " + v.getName() + (neg ? "'": "");
            r.push_back(rs);
            if (rs.size() > maxs[v.getIndex()]) maxs[v.getIndex()] = rs.size();
        }
        return r;
    }

    void Model::print() const {
        std::vector<std::vector<std::string>> cols;
        std::vector<size_t> maxs(variables.size(), 0);

        cols.reserve(constraints.size()+1);
        cols.push_back(print_expr(variables, &objc, maxs, built));
        std::cout << (objc.getObjectiveType() == ObjectiveFunction::ObjectiveType::Maximization ? "Maximize:\n" : "Minimize:\n");

        for (const auto& c : constraints) {
            cols.push_back(print_expr(variables, &c, maxs, built));
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
                const auto t = constraints[c_-1].getInequalityType();
                const auto b = built;
                if(b || t == Constraint::InequalityType::Equal) std::cout << " == ";
                else if(t == Constraint::InequalityType::LessOrEqual) std::cout << " <= ";
                else if(t == Constraint::InequalityType::GreaterOrEqual) std::cout << " >= ";
                std::cout << constraints[c_-1].getScalar() << "\n";
            } else {
                if (objc.getScalar() != 0) std::cout << (objc.getScalar() < 0 ? " - " : " + ") << abs(objc.getScalar());
                std::cout << "\n\n";
            }
        }

        std::cout << "\n";
        if (built) {
            for (const auto& v : variables) {
                std::cout << (v.getIndex() == 0 ? "" : ", ") << v.getName() << (v.getDomain() == Variable::Domain::NonPositive ? "'" : "");
            }
            std::cout << " >= 0\n";
        } else {
            std::string nneg = "", npos = "", unb = "";
            for (const auto& v : variables) {
                     if (v.getDomain() == Variable::Domain::NonNegative) nneg += ", " + v.getName();
                else if (v.getDomain() == Variable::Domain::NonPositive) npos += ", " + v.getName();
                else if (v.getDomain() == Variable::Domain::Unbounded)   unb += ", " + v.getName();
            }
            if (!nneg.empty()) std::cout << nneg.substr(2) << " >= 0\n";
            if (!npos.empty()) std::cout << npos.substr(2) << " <= 0\n";
            if (!unb.empty()) std::cout << unb.substr(2) << " unbounded\n";
        }
        std::cout << "\n";
    }

    Variable::Domain dual_constr_to_var_type(Constraint::InequalityType t) {
             if (t == Constraint::InequalityType::GreaterOrEqual) return Variable::Domain::NonNegative;
        else if (t == Constraint::InequalityType::LessOrEqual) return Variable::Domain::NonPositive;
        else return Variable::Domain::Unbounded;
    }

    Constraint::InequalityType dual_var_to_constr_type(Variable::Domain t) {
             if (t == Variable::Domain::NonNegative) return Constraint::InequalityType::LessOrEqual;
        else if (t == Variable::Domain::NonPositive) return Constraint::InequalityType::GreaterOrEqual;
        else return Constraint::InequalityType::Equal;
    }
    
    Model Model::getDual() const {
        if (isBuilt()) {
            Model tmp = *this;
            tmp.clear_artificials();
            return tmp.getDual();
        }
        Model dual;

        // SETS THE OBJECTIVE FUNCTION + VARIABLES
        if (dual.objective().getObjectiveType() == objc.getObjectiveType())
            dual.objective().multiplyBy(-1);
        
        for (const auto& c : constraints) {
            const auto v = dual.newVariable("y_" + c.getName(), dual_constr_to_var_type(c.getInequalityType()));
            dual.objective().setVariableCoefficient(v, c.getScalar());
        }

        // SETS THE CONSTRAINTS
        FOR_TO(v_i, variables.size()) {
            const auto& v = variables[v_i];
            auto dc = Constraint("dc_" + v.getName(), objc.getVariableCoefficient(v), dual_var_to_constr_type(v.getDomain()));
            FOR_TO(y_i, constraints.size()) {
                dc.setVariableCoefficient(dual.variables[y_i], constraints[y_i].getVariableCoefficient(v));
            }
            dual.add(dc);
        }

        return dual;
    }
}

#include <iostream>
#include "global.hpp"
#include "Model.hpp"

namespace Xplex {
    void Model::build() {
        std::cout << "Building model...\n";
        // TODO: Clear artificialObjective, c, b, A

        for (auto& c : constraints) {
            if (c.getScalar() < 0) c.multiplyBy(-1);
            if (c.getInequalityType() == Constraint::InequalityType::GreaterOrEqual) {
                const auto vi = OptIndex(variables.size());
                const auto n = "v"+std::to_string(vi)+"_cp"+std::to_string(c.getIndex());
                variables.push_back(Variable(n, vi, Variable::Type::NegativeSlack));
                c.setVariableCoefficient(variables[vi], -1.0);
            }
        }
        for (auto& c : constraints) {
            const bool slack = c.getInequalityType() == Constraint::InequalityType::LessOrEqual; // !slack => artificial
            const auto vi = OptIndex(variables.size());
            const auto n = "v"+std::to_string(vi)+(slack ? "_cs" : "_ca")+std::to_string(c.getIndex());
            variables.push_back(Variable(n, vi, (slack ? Variable::Type::Slack : Variable::Type::Artificial)));
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
            c_art(ci.first) = ci.second;
        }
        for (const auto& ci : objective().comp_variables) {
            c(ci.first) = ci.second;
        }

        for (const auto& c : constraints) {
            b(c.getIndex()) = c.getScalar();
            for (const auto& cv : c.comp_variables) {
                A(c.getIndex(), cv.first) = cv.second;
            }
        }

        built = true;
    }
    
    bool Model::isTwoPhaseNeeded() const {
        return !artificialObjective.comp_variables.empty();
    }
    
    bool Model::isBuilt() const {
        return built;
    }

    Model& Model::add(Variable& v) {
        v.index = OptIndex(variables.size());
        variables.push_back(v);
        return *this;
    }

    Model& Model::add(Constraint& c) {
        c.index = OptIndex(constraints.size());;
        constraints.push_back(c);
        #ifndef NDEBUG
        // constraints[index].dirty = false; // TODO: Complete
        #endif
        return *this;
    }

    Variable Model::newVariable(const std::string& name) {
        auto i = OptIndex(variables.size());
        auto v = Variable(name.empty() ? "v"+std::to_string(i) : name);
        v.index = i;
        variables.push_back(v);
        return v;
    }

    // Constraint&& Model::newConstraint(const std::string& name, double b_i, Constraint::InequalityType restr) {
    //     return add(Constraint(name, b_i, restr));
    // }
}

#include "Constraint.hpp"
#include "Variable.hpp"
#include <iostream>

namespace Xplex {

    Constraint::Constraint(const std::string& name, OptIndex index, double b_i, InequalityType restr, Type type)
        : name(name), index(index), b_i(b_i), restr(restr), type(type) {
        // #ifndef NDEBUG
        // dirty = false;
        // #endif
    }

    OptIndex Constraint::getIndex() const { return index; }

    const std::string& Constraint::getName() const { return name; }

    Constraint::Type Constraint::getType() const { return type; }

    Constraint::InequalityType Constraint::getInequalityType() const { return restr; }

    double Constraint::getVariableCoefficient(const Variable& v) const { return comp_variables.at(v.getIndex()); }

    double Constraint::getScalar() const { return b_i; }


    void Constraint::setScalar(double b) { b_i = b; }

    void Constraint::multiplyBy(double m) {
        for(auto& p : comp_variables) {
            p.second *= m;
        }
        b_i *= m;
        if (m < 0) {
            if (restr == InequalityType::GreaterOrEqual) restr = InequalityType::LessOrEqual;
            else if (restr == InequalityType::LessOrEqual) restr = InequalityType::GreaterOrEqual;
        }
    }

    void Constraint::setInequalityType(InequalityType t) {
        if (getType() == Type::Domain)
            throw std::runtime_error("Only valid to constraints of User type.");
        restr = t;
    }

    void Constraint::setVariableCoefficient(const Variable& v, double c) {
        if (v.getIndex() == -1)
            std::cerr << "WARNING: You must Model::add this variable before setting its coefficient.";
        comp_variables[v.getIndex()] = c;
    }
}

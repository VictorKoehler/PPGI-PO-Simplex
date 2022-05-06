#include "Constraint.hpp"
#include "Variable.hpp"
#include <iostream>

namespace Xplex {

    Constraint::Constraint(const Expression& e, double b, InequalityType restr) : Expression(e), name(""), index(-1), restr(restr), type(User) {
        setScalar(b - e.getScalar());
    }

    Constraint::Constraint(const std::string& name, OptIndex index, double b_i, InequalityType restr, Type type)
        : Expression(b_i), name(name), index(index), restr(restr), type(type) {
        // #ifndef NDEBUG
        // dirty = false;
        // #endif
    }

    OptIndex Constraint::getIndex() const { return index; }

    const std::string& Constraint::getName() const { return name; }

    Constraint::Type Constraint::getType() const { return type; }

    Constraint::InequalityType Constraint::getInequalityType() const { return restr; }


    void Constraint::multiplyBy(double m) {
        Expression::multiplyBy(m);
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



    Constraint operator <= (const Expression& e1, const double b) { return Constraint(e1, b, Constraint::InequalityType::LessOrEqual); }
    Constraint operator == (const Expression& e1, const double b) { return Constraint(e1, b, Constraint::InequalityType::Equal); }
    Constraint operator >= (const Expression& e1, const double b) { return Constraint(e1, b, Constraint::InequalityType::GreaterOrEqual); }

    Constraint operator <= (const Expression& e1, const Expression& e2) { return (e1-e2) <= 0.0; }
    Constraint operator == (const Expression& e1, const Expression& e2) { return (e1-e2) == 0.0; }
    Constraint operator >= (const Expression& e1, const Expression& e2) { return (e1-e2) >= 0.0; }

}

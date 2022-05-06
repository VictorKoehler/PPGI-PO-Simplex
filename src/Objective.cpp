#include "Objective.hpp"
#include <iostream>

namespace Xplex {

    ObjectiveFunction::ObjectiveFunction() : type(Maximization), originalType(Maximization) {};
    ObjectiveFunction::ObjectiveFunction(const Expression& e, ObjectiveType type) : Expression(e), type(type), originalType(type) {};

    ObjectiveFunction::ObjectiveType ObjectiveFunction::getObjectiveType() const { return type; }
    ObjectiveFunction::ObjectiveType ObjectiveFunction::getOriginalObjectiveType() const { return originalType; }
    const ObjectiveFunction ObjectiveFunction::getOriginalObjective() const {
        if (originalType == type) return *this;
        ObjectiveFunction o(*this);
        o.multiplyBy(-1);
        return o;
    }

    void ObjectiveFunction::multiplyBy(double m) {
        Expression::multiplyBy(m);
        if (m < 0) {
            type = type == ObjectiveType::Maximization ? ObjectiveType::Minimization : ObjectiveType::Maximization;
        }
    }

    ObjectiveFunction Minimize(const Expression& e) { return ObjectiveFunction(e, ObjectiveFunction::ObjectiveType::Minimization); }
    ObjectiveFunction Maximize(const Expression& e) { return ObjectiveFunction(e, ObjectiveFunction::ObjectiveType::Maximization); }
}

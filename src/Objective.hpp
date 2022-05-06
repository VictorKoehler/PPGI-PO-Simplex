#pragma once
#include "Expression.hpp"

namespace Xplex {
    class ObjectiveFunction : public Expression {
        friend class Model;
        public:
        enum ObjectiveType { Minimization, Maximization };

        protected:
        ObjectiveType type, originalType;

        public:
        ObjectiveFunction();
        ObjectiveFunction(const Expression& e, ObjectiveType type);

        ObjectiveType getObjectiveType() const;
        ObjectiveType getOriginalObjectiveType() const;
        const ObjectiveFunction getOriginalObjective() const;

        virtual void multiplyBy(double m) override;
    };

    ObjectiveFunction Minimize(const Expression& e);
    ObjectiveFunction Maximize(const Expression& e);
}
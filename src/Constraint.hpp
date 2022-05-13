#pragma once
#include <string>
#include <map>
#include "Expression.hpp"
#include "global.hpp"

namespace Xplex {
    class Constraint : public Expression {
        friend class Model;
        public:
        enum Type { User, Domain };
        enum InequalityType { LessOrEqual, Equal, GreaterOrEqual };

        protected:
        Constraint(const Expression& e, double b_i, InequalityType restr);

        Constraint(const std::string& name, OptIndex index, double b_i, InequalityType restr, Type type);
        
        Constraint(OptIndex index, double b_i, InequalityType restr, Type type)
            : Constraint("c"+std::to_string(index), index, b_i, restr, type) {}

        std::string name;
        OptIndex index;
        InequalityType restr;
        Type type;

        public:
        Constraint(const std::string& name, double b_i=0, InequalityType restr=LessOrEqual) : Constraint(name, -1, b_i, restr, User) {}

        /**
         * @brief Get the index of this variable on model. -1 if not indexed.
         */
        OptIndex getIndex() const;
        const std::string& getName() const;
        Type getType() const;
        InequalityType getInequalityType() const;
        
        void setInequalityType(InequalityType t);
        virtual void multiplyBy(double m) override;

        friend Constraint operator <= (const Expression& e1, const double b);
        friend Constraint operator == (const Expression& e1, const double b);
        friend Constraint operator >= (const Expression& e1, const double b);
        friend Constraint operator <= (const Expression& e1, const Expression& e2);
        friend Constraint operator == (const Expression& e1, const Expression& e2);
        friend Constraint operator >= (const Expression& e1, const Expression& e2);
    };

    Constraint operator <= (const Expression& e1, const double b);
    Constraint operator == (const Expression& e1, const double b);
    Constraint operator >= (const Expression& e1, const double b);
    Constraint operator <= (const Expression& e1, const Expression& e2);
    Constraint operator == (const Expression& e1, const Expression& e2);
    Constraint operator >= (const Expression& e1, const Expression& e2);
}

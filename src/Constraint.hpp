#pragma once
#include <string>
#include <map>
#include "Variable.hpp"
#include "global.hpp"

namespace Xplex {
    class Constraint {
        friend class Model;
        public:
        enum Type { User, Domain, Objective };
        enum InequalityType { LessOrEqual, Equal, GreaterOrEqual };

        const static InequalityType MINIMIZE = InequalityType::LessOrEqual;
        const static InequalityType MAXIMIZE = InequalityType::GreaterOrEqual;
        
        protected:
        Constraint(const std::string& name, OptIndex index, double b_i, InequalityType restr, Type type);
        
        Constraint(OptIndex index, double b_i, InequalityType restr, Type type)
            : Constraint("c"+std::to_string(index), index, b_i, restr, type) {}
        
        Constraint() : Constraint("Obj", -1, 0, MAXIMIZE, Objective) {}

        std::string name;
        OptIndex index;
        double b_i;
        InequalityType restr;
        Type type;
        std::map<OptIndex, double> comp_variables;
        // #ifndef NDEBUG // TODO: dirty
        // bool dirty;
        // #endif

        public:
        Constraint(const std::string& name, double b_i=0, InequalityType restr=LessOrEqual) : Constraint(name, -1, b_i, restr, User) {}

        /**
         * @brief Get the index of this variable on model. -1 if not indexed.
         */
        OptIndex getIndex() const;
        const std::string& getName() const;
        Type getType() const;
        InequalityType getInequalityType() const;
        double getVariableCoefficient(const Variable& v) const;
        double getScalar() const;
        
        void setInequalityType(InequalityType t);
        void setVariableCoefficient(const Variable& v, double c);
        void setScalar(double b);
        void multiplyBy(double m);
    };
}

#pragma once
#include <string>
#include <map>
#include "global.hpp"

namespace Xplex {
    class Constraint {
        friend class Model;
        public:
        enum Type { User, Domain, Objective };
        enum InequalityType { LessOrEqual, Equal, GreaterOrEqual };
        
        protected:
        Constraint(const std::string& name, OptIndex index, double b_i, InequalityType restr, Type type)
            : name(name), index(index), b_i(b_i), restr(restr), type(type) {
            #ifndef NDEBUG
            dirty = false;
            #endif
        }
        
        Constraint(OptIndex index, double b_i, InequalityType restr, Type type)
            : Constraint("c"+std::to_string(index), index, b_i, restr, type) {}
        
        Constraint() : Constraint("Minimize", -1, 0, Equal, Objective) {}

        std::string name;
        OptIndex index;
        double b_i;
        InequalityType restr;
        Type type;
        std::map<OptIndex, double> comp_variables;
        #ifndef NDEBUG
        bool dirty;
        #endif

        public:
        Constraint(const std::string& name, double b_i=0, InequalityType restr=LessOrEqual) : Constraint(name, -1, b_i, restr, User) {}

        /**
         * @brief Get the index of this variable on model. -1 if not indexed.
         */
        inline OptIndex getIndex() const { return index; }
        inline const std::string& getName() const { return name; }
        inline Type getType() const { return type; }
        inline InequalityType getInequalityType() const { return restr; }
        inline double getVariableCoefficient(const Variable& v) const { return comp_variables.at(v.getIndex()); }
        inline double getScalar() const { return b_i; }
        
        inline void setInequalityType(InequalityType t) {
            if (getType() != Type::User)
                throw std::runtime_error("Only valid to constraints of User type.");
            restr = t;
        }

        inline void setVariableCoefficient(const Variable& v, double c) {
            if (v.getIndex() == -1)
                std::cerr << "WARNING: You must Model::add this variable before setting its coefficient.";
            std::cout << "ADDING " << v.getIndex() << " " << c << " to " << getIndex() << "(" << this << ")\n";
            comp_variables[v.getIndex()] = c;
        }

        inline void setScalar(double b) { b_i = b; }
    };
}

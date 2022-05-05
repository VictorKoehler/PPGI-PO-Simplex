#pragma once
#include <string>
#include <vector>
#include "Constraint.hpp"

namespace Xplex {
    class Model {
        friend class Xplex;

        protected:
        bool built;
        Constraint objective, artificialObjective; // We only Maximize!
        std::vector<Variable> variables;
        std::vector<Constraint> constraints;
        VectorXd c, c_art, b;
        MatrixXd A;

        public:
        Model() { }

        void build();
        
        bool isTwoPhaseNeeded() const;
        bool isBuilt() const;

        void add(Variable& v);
        void add(Constraint& c);

        Variable newVariable(const std::string& name="");

        inline Constraint& objectiveFunction() { return objective; }
    };
}

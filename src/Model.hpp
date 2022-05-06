#pragma once
#include <string>
#include <vector>
#include "Constraint.hpp"
#include "Objective.hpp"

namespace Xplex {
    class Model {
        friend class Xplex;

        protected:
        bool built;
        ObjectiveFunction objc, artificialObjective; // We only Maximize!
        std::vector<Variable> variables;
        std::vector<Constraint> constraints;
        VectorXd c, c_art, b;
        MatrixXd A;

        public:
        Model() { }

        void build();
        
        bool isTwoPhaseNeeded() const;
        bool isBuilt() const;

        Model& add(Variable& v);
        Model& add(Constraint& c);

        Variable newVariable(const std::string& name="");

        inline ObjectiveFunction& objective() { return objc; }
    };
}

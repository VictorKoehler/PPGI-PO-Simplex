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

        void clear_artificials();

        public:
        Model() : built(false) { }
        
        bool isTwoPhaseNeeded() const;
        bool isBuilt() const;

        void print() const;


        void build();

        Model& add(Variable& v);
        Model& add(Constraint& c);


        Model& add_discard(const Constraint& copy_discarded);

        Variable newVariable(const std::string& name="", Variable::Domain domain = Variable::Domain::NonNegative);

        inline ObjectiveFunction& objective() { return objc; }
    };
}

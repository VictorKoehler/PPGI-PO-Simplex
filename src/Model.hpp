#pragma once
#include <string>
#include <vector>
#include "Variable.hpp"
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
        Model() { // TODO: ERASE
            // built = true;
            // for (int i = 0; i < 6; i++) {
            //     variables.push_back(Variable("x_"+std::to_string(i+1), i, Variable::Type::User));
            // }
            // c.resize(6);
            // b.resize(3);
            // c << 1, 2, -1, 0, 0, 0;
            // b << 14, 28, 30;
            // A = MatrixXd({
            //     {2, 1, 1, 1, 0, 0},
            //     {4, 2, 3, 0, 1, 0},
            //     {2, 5, 5, 0, 0, 1}
            // });
        }

        void build();
        
        bool isTwoPhaseNeeded() const;
        bool isBuilt() const;

        void add(Variable& v);
        void add(Constraint& c);

        Variable newVariable(const std::string& name="");

        inline Constraint& objectiveFunction() { return objective; }
    };
}

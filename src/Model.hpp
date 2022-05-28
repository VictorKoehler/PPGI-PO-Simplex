#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include "Constraint.hpp"
#include "Objective.hpp"

namespace Xplex {
    template<typename C>
    class RefToElemOfUnshrikableContainer {
        protected:
        size_t ind;
        C* container;
        public:
        RefToElemOfUnshrikableContainer(size_t ind, C* container) : ind(ind), container(container) {}
        RefToElemOfUnshrikableContainer() : ind(0), container(nullptr) {}

        bool has_value() const noexcept { return container != nullptr; }
        auto* get() const { return &(*container)[ind]; }
        auto* operator->() const { return &(*container)[ind]; }
        auto& operator*() const { return (*container)[ind]; }
    };

    class Model {
        friend class Xplex;

        protected:
        bool built;
        ObjectiveFunction objc, artificialObjective; // We only Maximize!
        std::vector<Variable> variables;
        std::vector<Constraint> constraints;
        VectorXd c, c_art, b;
        MatrixXd A;

        std::unordered_map<std::string, OptIndex> variables_names;

        void clear_artificials();

        public:
        typedef RefToElemOfUnshrikableContainer<std::vector<Variable>> VariableRef;
        Model() : built(false) { }
        
        bool isTwoPhaseNeeded() const;
        bool isBuilt() const;
        Model getDual() const;
        void print(bool spaced=true) const;


        void build();

        Model& add(Variable& v);
        Model& add(Constraint& c);

        Model& add_discard(const Constraint& copy_discarded);

        VariableRef newVariable(const std::string& name="", Variable::Domain domain = Variable::Domain::NonNegative);

        VariableRef getVariable(const std::string& name);
        inline ObjectiveFunction& objective() { return objc; }

        const std::vector<Variable>& getVariables() const { return variables; }
        const std::vector<Constraint>& getConstraints() const { return constraints; }
    };
}

#pragma once
#include "Model.hpp"

namespace Xplex {
    class Xplex {
        protected:
        const Model *model;
        uint iterations;
        bool verbose;
        MatrixXd A_B_m1; // aka B_^{-1}
        VectorXd xc_b; // Values of basic variables = A_B_m1*b, same indexing as basic_vars
        std::vector<uint> basic_vars; // where j=basic_vars[i] points to model->variables[j]
        std::vector<bool> variable_is_basic; // where variable_is_basic[i] points to model->variables[j]

        void revised_simplex();

        public:
        Xplex(const Model *model) : model(model), iterations(0), verbose(false) {}

        void solve();

        inline size_t getBasisSize() const { return basic_vars.size(); } // aka m
        inline size_t getNonBasisSize() const { return model->variables.size() - getBasisSize(); } // aka nn
        inline bool isVerbose() const { return verbose; }
        inline void setVerbose(bool verb) { verbose = verb; }
    };
}
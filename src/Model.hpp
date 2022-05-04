#pragma once
#include <Eigen/Dense>
#include <string>
#include <vector>

namespace Xplex {
    using Eigen::MatrixXd;
    using Eigen::VectorXd;

    class Xplex;
    class Model;

    class Variable {
        friend class Model;
        // public:
        // enum Type {
        //     User, Slack
        // };
        
        public:
        Variable(std::string name, int index) : name(name), index(index) {}
        std::string name;
        // Type type;
        int index;

        public:
        Variable(std::string name) : Variable(name, -1) {}

        inline const std::string& getName() const { return name; }
        // inline Type getType() const { return type; }

        /**
         * @brief Get the index of this variable on model. -1 if not indexed.
         */
        inline int getIndex() const { return index; }
    };

    class Model {
        friend class Xplex;

        protected:
        std::vector<Variable> variables;
        VectorXd c, b;
        MatrixXd A;
    };
}

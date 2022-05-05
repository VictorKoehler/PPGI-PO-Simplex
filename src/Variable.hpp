#pragma once
#include <string>
#include "global.hpp"

namespace Xplex {
    class Variable {
        friend class Model;
        public:
        enum Type { User, Slack, Artificial, NegativeSlack };
        
        protected:
        Variable(const std::string& name, OptIndex index, Type type) : name(name), index(index), type(type) {}
        Variable(OptIndex index, Type type) : Variable("v"+std::to_string(index), index, type) {}
        std::string name;
        OptIndex index;
        Type type;
        // TODO: Domain

        public:
        Variable(const std::string& name) : Variable(name, -1, User) {}

        inline const std::string& getName() const { return name; }
        inline Type getType() const { return type; }

        /**
         * @brief Get the index of this variable on model. -1 if not indexed.
         */
        inline OptIndex getIndex() const { return index; }
    };
}

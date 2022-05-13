#pragma once
#include <string>
#include "global.hpp"

namespace Xplex {
    class Variable {
        friend class Model;
        public:
        enum Type { User, Slack, Artificial, NegativeSlack, NegativeMirror };
        enum Domain { NonPositive, NonNegative, Unbounded };
        
        protected:
        Variable(const std::string& name, OptIndex index, Type type, Domain domain) : name(name), index(index), type(type), domain(domain) {}
        Variable(OptIndex index, Type type, Domain domain) : Variable("v"+std::to_string(index), index, type, domain) {}
        std::string name;
        OptIndex index;
        Type type;
        Domain domain;

        public:
        Variable(const std::string& name, Domain domain = NonNegative) : Variable(name, -1, User, domain) {}

        inline const std::string& getName() const { return name; }
        inline Type getType() const { return type; }
        inline Domain getDomain() const { return domain; }

        /**
         * @brief Get the index of this variable on model. -1 if not indexed.
         */
        inline OptIndex getIndex() const { return index; }

        inline void setDomain(Domain d) { domain = d; }
    };
}

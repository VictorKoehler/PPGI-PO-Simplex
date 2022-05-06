#pragma once
#include <map>
#include "Variable.hpp"

namespace Xplex {
    class Expression {
        protected:
        double scalar;
        std::map<OptIndex, double> comp_variables;
        Expression(const double m, const Variable &v) : scalar(0) {
            setVariableCoefficient(v, m);
        }

        public:
        Expression(double scalar = 0) : scalar(scalar) {}

        double getVariableCoefficient(const Variable& v) const;
        double getScalar() const;

        const std::map<OptIndex, double>& getVariablesMap() const;

        void setVariableCoefficient(const Variable& v, double c);
        void setScalar(double b);
        virtual void multiplyBy(double m);

        friend Expression operator * (const double m, const Variable &v);
        friend Expression operator + (const Expression& m, const Variable &v);
        friend Expression operator + (const Variable &v, const Expression& m);
        friend Expression operator + (const Expression& e1, const Expression& e2);
        friend Expression operator - (const Expression& e1, const Expression& e2);
    };

    Expression operator * (const double m, const Variable &v);
    Expression operator + (const Expression& m, const Variable &v);
    Expression operator + (const Variable &v, const Expression& m);
    Expression operator + (const Variable &v1, const Variable &v2);
    Expression operator + (const Expression& e1, const Expression& e2);
    Expression operator - (const Expression& e1, const Expression& e2);
}
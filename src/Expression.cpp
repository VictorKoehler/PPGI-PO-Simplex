#include "Expression.hpp"
#include "Variable.hpp"
#include <iostream>

namespace Xplex {

    double Expression::getVariableCoefficient(const Variable& v) const {
        if (v.getIndex() == -1)
            std::cerr << "WARNING: You must Model::add this variable before setting its coefficient.";
        auto f = comp_variables.find(v.getIndex());
        if (f == comp_variables.end()) return 0;
        else return f->second;
    }

    double Expression::getScalar() const { return scalar; }

    const std::map<OptIndex, double>& Expression::getVariablesMap() const { return comp_variables; }


    void Expression::setScalar(double b) { scalar = b; }

    void Expression::multiplyBy(double m) {
        for(auto& p : comp_variables) {
            p.second *= m;
        }
        scalar *= m;
    }

    void Expression::setVariableCoefficient(const Variable& v, double c) {
        if (v.getIndex() == -1)
            std::cerr << "WARNING: You must Model::add this variable before setting its coefficient.";
        comp_variables[v.getIndex()] = c;
    }


    Expression operator * (const double m, const Variable &v) {
        return Expression(m, v);
    }

    Expression operator + (const Expression& m, const Variable &v) {
        return m + 1.0*v;
    }

    Expression operator + (const Variable &v, const Expression& m) {
        return m + 1.0*v;
    }

    Expression operator + (const Variable &v1, const Variable &v2) {
        return 1.0*v1 + v2;
    }

    Expression operator + (const Expression& e1, const Expression& e2) {
        Expression r(e1.getScalar() + e2.getScalar());
        for (auto const& [v, m] : e1.comp_variables) {
            r.comp_variables[v] = m;
        }
        for (auto const& [v, m] : e2.comp_variables) {
            auto f = r.comp_variables.find(v);
            r.comp_variables[v] = (f == r.comp_variables.end() ? 0 : f->second) + m;
        }
        return r;
    }

    Expression operator - (const Expression& m, const Variable &v) {
        return m + (-1.0)*v;
    }

    Expression operator - (const Variable &v, const Expression& m) {
        // return 1.0*v + (-1.0)*m; // Not defined
        return 1.0*v - m;
    }

    Expression operator - (const Variable &v1, const Variable &v2) {
        return 1.0*v1 + (-1.0)*v2;
    }


    Expression operator - (const Expression& e1, const Expression& e2) {
        Expression r(e1.getScalar() - e2.getScalar());
        for (auto const& [v, m] : e1.comp_variables) {
            r.comp_variables[v] = m;
        }
        for (auto const& [v, m] : e2.comp_variables) {
            auto f = r.comp_variables.find(v);
            r.comp_variables[v] = (f == r.comp_variables.end() ? 0 : f->second) - m;
        }
        return r;
    }
}

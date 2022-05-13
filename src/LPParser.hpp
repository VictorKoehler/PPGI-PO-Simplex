#pragma once
#include <string>
#include <unordered_map>
#include <istream>

#include <tuple>
#include <vector>
#include "Model.hpp"

namespace Xplex {
    class LPParserBase {
        public:
        typedef Constraint::InequalityType InequalityType;
        typedef ObjectiveFunction::ObjectiveType ObjectiveType;
        // enum ObjectiveType { Minimization, Maximization }; // desacoplado
        // enum InequalityType { LessOrEqual, Equal, GreaterOrEqual };

        LPParserBase(const std::string& filepath);
        LPParserBase(std::istream* input);
        ~LPParserBase();
        
        virtual void parse();

        protected:
        std::istream* input;
        bool delete_on_exit, objdone, symbol_restored;
        std::string expr_name, last_symbol;

        std::unordered_map<std::string, double> coeff, obj;

        const std::string& consume_symbol();
        void restore_symbol();
        void restore_symbol(const std::string& custom_symbol);

        void _read_expr_var_reg(const std::string& varname, double value);
        void _read_obj();
        void _read_expr();
        void _read_ineq();
        void _read_bounds();

        virtual void add_variable(const std::string& n) = 0;
        virtual void set_variable_bounds(const std::string& n, double lb, double ub) = 0;
        virtual void set_objective(ObjectiveType t, std::unordered_map<std::string, double>& coeff) = 0;
        virtual void add_constraint(std::unordered_map<std::string, double>& coeff, InequalityType t, double b, const std::string& n = "") = 0;
    };


    // Versão acoplada ao Xplex
    class LPParserXplexModel : public LPParserBase {
        protected:
        Model& m;
        std::unordered_map<std::string, Variable> vars;

        virtual void add_variable(const std::string& n) override;
        virtual void set_variable_bounds(const std::string& n, double lb, double ub) override;
        virtual void set_objective(ObjectiveType t, std::unordered_map<std::string, double>& coeff) override;
        virtual void add_constraint(std::unordered_map<std::string, double>& coeff, InequalityType t, double b, const std::string& n = "") override;

        public:
        LPParserXplexModel(Model& m, const std::string& filepath);
        LPParserXplexModel(Model& m, std::istream* input);
    };
}

#include <iostream>
#include <fstream>
#include <unordered_map>
#include "LPParser.hpp"

namespace Xplex {

    typedef LPParserBase::ObjectiveType ObjType;

    LPParserBase::LPParserBase(const std::string& filepath) {
        input = new std::ifstream(filepath, std::ios_base::in);
        delete_on_exit = true;
        symbol_restored = false;
        objdone = false;
    }

    LPParserBase::LPParserBase(std::istream* input) : input(input) {
        delete_on_exit = false;
        symbol_restored = false;
        objdone = false;
    }

    LPParserBase::~LPParserBase() {
        if (delete_on_exit) delete input;
    }


    // from https://stackoverflow.com/a/25385766, because C++ is disgustingly incomplete
    const char* ws = " \t\n\r\f\v";
    // trim from end of string (right)
    inline std::string& rtrim(std::string& s, const char* t = ws) {
        s.erase(s.find_last_not_of(t) + 1);
        return s;
    }

    // trim from beginning of string (left)
    inline std::string& ltrim(std::string& s, const char* t = ws) {
        s.erase(0, s.find_first_not_of(t));
        return s;
    }

    // trim from both ends of string (right then left)
    inline std::string& trim(std::string& s, const char* t = ws) {
        return ltrim(rtrim(s, t), t);
    }

    // trim from both ends of string (right then left)
    inline std::string ctrim(const std::string& s_, const char* t = ws) {
        auto s = s_;
        return ltrim(rtrim(s, t), t);
    }

    inline std::string& str_to_uppercase(std::string& s) {
        for (auto & c: s) c = toupper(c);
        return s;
    }

    inline std::string str_to_uppercase(const std::string& s) {
        std::string s1 = s;
        s1 = str_to_uppercase(s1);
        return s1;
    }


    static const std::string CHARS_ALLOWED_IN_NAMES = "abcdfghijklmnopqrstuvwxyzABCDFGHIJKLMNOPQRSTUVWXYZ.eE0123456789!#$%&(),;?’@_{}~‘\"";
    static const std::string CHAR_ALLOWED_IN_0POS_OF_NAME = "abcdfghijklmnopqrstuvwxyzABCDFGHIJKLMNOPQRSTUVWXYZ!#$%&(),;?’@_{}~‘\"";
    static const std::string CHAR_NUMBER = ".0123456789";

    auto _parser_getline(std::istream* input, bool to_upper=false) {
        std::string line;
        while (input->good()) {
            std::getline(*input, line);
            const auto cstart = line.find_first_of("\\");
            if (cstart != line.npos) line.erase(cstart);
            trim(line);
            if (to_upper) str_to_uppercase(line);
            if (!line.empty()) return line;
            // std::stringstream ss(line);
        }
        return std::string("");
    }

    const std::string& LPParserBase::consume_symbol() {
        if (symbol_restored) symbol_restored = false;
        else last_symbol = _parser_getline(input);
        // std::cout << "CONSUME: " << last_symbol << "\n";
        return last_symbol;
    }

    void LPParserBase::restore_symbol() {
        symbol_restored = true;
    }

    void LPParserBase::restore_symbol(const std::string& custom_symbol) {
        symbol_restored = true;
        last_symbol = custom_symbol;
    }



    void LPParserBase::_read_obj() {
        const auto mi = ObjType::Minimization, ma = ObjType::Maximization;
        std::unordered_map<std::string, ObjType> types = {{"MINIMIZE", mi}, {"MINIMUM", mi}, {"MIN", mi}, {"MAXIMIZE", ma}, {"MAXIMUM", ma}, {"MAX", ma}};
        auto sense = types.find(str_to_uppercase(consume_symbol()))->second;
        
        // std::cout << "SENSE: " << sense << "\n";
        _read_expr();

        obj.swap(coeff);
        set_objective(sense, coeff);
        objdone = true;
        coeff.clear();
    }

    std::string read_signal(const std::string& line) {
        const auto s = line.substr(0, 1);
        return s == "+" || s == "-" ? s : "";
    }

    std::string read_number(const std::string& line, bool verify_sign=false) {
        bool has_sign = verify_sign && (line[0] == '-' || line[0] == '+');
        auto s = line.find_first_not_of(CHAR_NUMBER, has_sign ? 1 : 0);
        if (s == line.npos) s = line.length();
        else if (CHAR_ALLOWED_IN_0POS_OF_NAME.find(line[s]) != std::string::npos) return "";
        return line.substr(0, s);
    }

    std::string read_number_or_inf(const std::string& line, bool verify_sign=true) {
        const auto r = read_number(line, verify_sign);
        if (!r.empty()) return r;
        if (line[0] != '-' && line[0] != '+') return "";
        const auto si = line.find_first_not_of(CHARS_ALLOWED_IN_NAMES, 1);
        const auto s = str_to_uppercase(line.substr(0, si));
        return s.substr(1) == "INFINITY" || s.substr(1) == "INF" ? s : "";
    }

    std::string read_name(const std::string& line) {
        if (CHAR_ALLOWED_IN_0POS_OF_NAME.find(line[0]) == std::string::npos) return "";
        auto s = line.find_first_not_of(CHARS_ALLOWED_IN_NAMES, 1);
        if (s == line.npos) s = line.length();
        return line.substr(0, s);
    }
    
    void LPParserBase::_read_expr() {
        expr_name = "";
        std::string line = consume_symbol();
        const auto expr_name_pos = line.find(':');
        if (expr_name_pos != line.npos) {
            expr_name = line.substr(0, expr_name_pos);
            line = line.substr(expr_name_pos+1);
            trim(line);
        }
        // std::cout << "Reading expression of name '" << expr_name << "'\n";
        
        double number = 1;
        bool signal_is_positive = true, state_signal = true, state_number = true, state_name = true;
        while (true) {
            // int st = 0;
            // std::cout << "RE: " << state_signal << state_number << state_name << " ";

            std::string s;
            if (state_signal && !(s=read_signal(line)).empty()) {
                signal_is_positive = s == "+";
                state_signal = false;
                state_number = state_name = true;
            } else if (state_number && !(s=read_number(line)).empty()) {
                number = std::atof(s.c_str());
                state_signal = state_number = false;
            } else if (state_name && !(s=read_name(line)).empty()) {
                _read_expr_var_reg(s, number * (signal_is_positive ? 1 : -1));
                number = 1;
                state_signal = true;
                state_number = state_name = false;
            } else {
                restore_symbol(line);
                break;
            }

            line = line.substr(s.length());
            ltrim(line);
            // std::cout << st << " #" << s << "#" << line << "\n";
            if (line.empty()) {
                line = consume_symbol();
                ltrim(line);
            }
        }
    }

    void LPParserBase::_read_expr_var_reg(const std::string& varname, double value) {
        if (obj.find(varname) == obj.end()) {
            add_variable(varname);
            obj[varname] = objdone ? 0 : value;
        }
        const auto p = coeff.find(varname);
        coeff[varname] = value + (p == coeff.end() ? 0 : p->second);
        // std::cout << "COEFF_SET: " << varname << " -> " << coeff[varname] << "\n";
    }

    void LPParserBase::_read_ineq() {
        auto st = str_to_uppercase(consume_symbol());
        assert(st == "SUBJECT TO" || st == "SUCH THAT" || st == "ST" || st == "S.T.");
        int cc = 0;
        while (true) {
            cc++;
            st = str_to_uppercase(consume_symbol());
            restore_symbol();
            if (st == "END" || st == "BOUND" || st == "BOUNDS") break;

            _read_expr();
            st = consume_symbol();
            if (st.empty()) throw std::runtime_error("Expression malformed: " + expr_name);
            InequalityType t;
            const auto v2 = st.substr(0, 2);
            if (v2.size() >= 2 && v2[1] == '=') {
                if (v2 == "<=") t = LPParserBase::InequalityType::LessOrEqual;
                else if (v2 == ">=") t = LPParserBase::InequalityType::GreaterOrEqual;
                else if (v2 == "==") t = LPParserBase::InequalityType::Equal;
                else throw std::runtime_error("Expression malformed: " + st);
                st = st.substr(2);
            } else if (v2[0] == '=') {
                t = LPParserBase::InequalityType::Equal;
                st = st.substr(1);
            } else {
                throw std::runtime_error("Expression malformed: " + st);
            }
            trim(st);

            if (coeff.empty()) std::cerr << "WARNING: Empty constraint '" << (expr_name.empty() ? "*" : "") << "' (" << cc << ") \n";
            add_constraint(coeff, t, std::atof(st.c_str()), expr_name);
            coeff.clear();
        }
    }

    struct TMPExpression {
        struct TMPTerm {
            std::string t;
            bool name;

            TMPTerm() : t(""), name(false) {}
            bool read(const std::string& t) {
                this->name = false;
                this->t = read_number_or_inf(t);
                if (this->t.empty()) {
                    this->t = read_name(t);
                    if (!this->t.empty()) {
                        this->name = true;
                    }
                }
                return valid();
            }

            void reset() { t = ""; name = false; }
            bool valid() const { return !t.empty(); }
            bool isName() const { return valid() && name; }
            // void print() const { std::cout << (isName() ? "N'" : (valid() ? "V'" : "I'")) << t << "' "; }
        } first, second, third;

        std::string next_ineq(const std::string& l, bool throw_invalid=true, bool throw_blank=true) {
            if (l.empty()) {
                if (throw_blank) throw std::runtime_error("Expected an inequality symbol. Got nothing.");
                return "";
            } else if (l[0] == '=') return l.size() >= 2 && l[1] == '=' ? "==" : "=";
            else if (l.substr(0, 2) == "<=") return "<=";
            else if (l.substr(0, 2) == ">=") return ">=";
            else if (throw_invalid) throw std::runtime_error("Expected an inequality symbol. Got '"+l+"'.");
            else return "";
        }

        std::string pad_eq(const std::string& i) {
            return i.size() == 1 ? i+"=" : i;
        }

        void read_full_line(const std::string& l_) {
            const auto l1 = read_line(l_);
            if (!l1.empty()) {
                throw std::runtime_error("Expression wasn't completely cleared: '" + l_ + "' -> '" + l1 + "'.");
            }
        }

        std::string read_line(const std::string& l_) {
            auto l = l_;
            if (!first.read(l)) return l_;
            l = ctrim(l.substr(first.t.length()));

            const auto op1 = next_ineq(l);
            l = ctrim(l.substr(op1.length()));
            
            if(!second.read(l)) throw std::runtime_error("Invalid expression while reading 2nd term: "+l_);
            l = ctrim(l.substr(second.t.length()));

            if (op1 == ">=") {
                third = first;
                first.reset();
                if (third.isName()) {
                    first = second;
                    second = third;
                    third.reset();
                }
            } else if (op1[0] == '=') {
                if (!second.isName()) std::swap(first, second);
                return l;
            } else if (first.isName()) { // <=
                third = second;
                second = first;
                first.reset();
            }

            const auto op2 = next_ineq(l, false, false);
            if (op2.empty()) return l;
            l = ctrim(l.substr(op1.length()));
            if (pad_eq(op1) != pad_eq(op2)) throw std::runtime_error("Inequality cannot have distinct symbols: "+l_);
            
            auto& plast = first.valid() ? third : first;
            if (!plast.read(l)) throw std::runtime_error("Invalid expression while reading 3nd term: "+l_);
            l = ctrim(l.substr(plast.t.length()));

            if ((first.valid() || third.valid()) && (!first.isName() && second.isName() && !third.isName())) {
                return l;
            }
            throw std::runtime_error("Could not determine expression validity: " + l_);
        }
    };
    
    void LPParserBase::_read_bounds() {
        auto k = str_to_uppercase(consume_symbol());
        if (k != "BOUND" && k != "BOUNDS") return;
        std::unordered_map<std::string, double> bounds_ub, bounds_lb;
        while (true) {
            k = consume_symbol();
            if (str_to_uppercase(k) == "END") break;
            std::string kn;
            double lb = NAN, ub = NAN;

            if (str_to_uppercase(k).find(" FREE") != k.npos) {
                kn = read_name(k);
                lb = -INFINITY;
                ub = +INFINITY;
            } else {
                TMPExpression ev;
                ev.read_full_line(k);
                if (ev.first.valid()) lb = std::atof(ev.first.t.c_str());
                kn = ev.second.t;
                if (ev.third.valid()) ub = std::atof(ev.third.t.c_str());
                // ev.first.print();
                // ev.second.print();
                // ev.third.print();
                // std::cout << "\n";
            }

            if (kn.empty()) throw std::runtime_error("read_name failed to find variable name: " + last_symbol);
            if (obj.find(kn) == obj.end()) std::cerr << "WARNING: Defining bounds to a non defined variable: " << kn << "\n";
            if (!std::isnan(ub)) bounds_ub[kn] = ub;
            if (!std::isnan(lb)) bounds_lb[kn] = lb;
            // set_variable_bounds(kn, lb, ub);
        }

        for (const auto [kn, kub] : bounds_ub) {
            auto lb_it = bounds_lb.find(kn);
            if (lb_it == bounds_lb.end()) {
                const auto dlb = kub > 0 ? 0 : -INFINITY;
                set_variable_bounds(kn, dlb, kub);
            } else {
                set_variable_bounds(kn, lb_it->second, kub);
                bounds_lb.erase(lb_it);
            }
        }
        
        for (const auto [kn, klb] : bounds_lb) {
            set_variable_bounds(kn, klb, +INFINITY);
        }
    }

    void LPParserBase::parse() {
        _read_obj();
        _read_ineq();
        _read_bounds();
    }



    void LPParserXplexModel::add_variable(const std::string& n) {
        m.newVariable(n);
    }

    void LPParserXplexModel::set_variable_bounds(const std::string& n, double lb, double ub) {
        // std::cout << "BOUNDS: " << lb << " <= " << n << " <= " << ub << "\n";
        auto ov = m.getVariable(n);
        if (!ov.has_value()) return;
        auto b = Variable::Domain::NonNegative;
        if (lb != 0) {
            if (lb < 0) {
                b = ub > 0 ? Variable::Domain::Unbounded : Variable::Domain::NonPositive;
            }
            if (lb != -INFINITY) m.add_discard(1.0*(*ov) >= lb); // TODO: Variable class doesn't support defining numerical bounds
        }
        if (ub != +INFINITY) m.add_discard(1.0*(*ov) <= ub);
        ov->setDomain(b);
    }

    void LPParserXplexModel::set_objective(ObjectiveType t, std::unordered_map<std::string, double>& coeff) {
        m.objective() = ObjectiveFunction(Expression(), t);
        m.objective().setScalar(0);
        for (const auto& [vkey, vcoeff] : coeff) {
            m.objective().setVariableCoefficient(*m.getVariable(vkey), vcoeff);
        }
    }

    void LPParserXplexModel::add_constraint(std::unordered_map<std::string, double>& coeff, InequalityType t, double b, const std::string& n) {
        if (coeff.empty()) {
            std::cerr << "Aborted atempt to insert an empty constraint: " << n << "\n";
            return;
        }
        Constraint c(n, b, t);
        for (const auto& [vkey, vcoeff] : coeff) {
            c.setVariableCoefficient(*m.getVariable(vkey), vcoeff);
        }
        m.add(c);
    }


    LPParserXplexModel::LPParserXplexModel(Model& m, const std::string& filepath) : LPParserBase(filepath), m(m) {}

    LPParserXplexModel::LPParserXplexModel(Model& m, std::istream* input) : LPParserBase(input), m(m) {}

}

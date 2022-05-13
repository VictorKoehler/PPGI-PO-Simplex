#include <iostream>
#include <fstream>
#include "Xplex.hpp"

int DEFAULT_VERBOSITY;

void example_1() {
    std::cout << "===== EXAMPLE 1 =====\n";
    Xplex::Model m;
    auto x1 = *m.newVariable("x1");
    auto x2 = *m.newVariable("x2");
    auto x3 = *m.newVariable("x3");
    auto c1 = Xplex::Constraint("c1", 14);
    auto c2 = Xplex::Constraint("c2", 28);
    auto c3 = Xplex::Constraint("c3", 30);
    std::cout << x1.getIndex() << " " << x2.getIndex() << " " << x3.getIndex() << " "
              << c1.getIndex() << " " << c2.getIndex() << " " << c3.getIndex() << "\n";
    c1.setVariableCoefficient(x1, 2);
    c1.setVariableCoefficient(x2, 1);
    c1.setVariableCoefficient(x3, 1);
    c2.setVariableCoefficient(x1, 4);
    c2.setVariableCoefficient(x2, 2);
    c2.setVariableCoefficient(x3, 3);
    c3.setVariableCoefficient(x1, 2);
    c3.setVariableCoefficient(x2, 5);
    c3.setVariableCoefficient(x3, 5);
    m.add(c1);
    m.add(c2);
    m.add(c3);
    m.objective().setVariableCoefficient(x1, 1);
    m.objective().setVariableCoefficient(x2, 2);
    m.objective().setVariableCoefficient(x3, -1);
    m.print();
    m.build();

    Xplex::Xplex xplex(&m);
    xplex.setVerbose(DEFAULT_VERBOSITY >= 1);
    xplex.solve();
    std::cout << xplex.getObjValue() << ": " << xplex.getValue(x1) << " " << xplex.getValue(x2) << " " << xplex.getValue(x3) << "\n";

    std::cout << "Expected x1=5 x2=4\n";
}

void example_2() { // Exemplo Dual Livro p85
    std::cout << "\n\n\n===== EXAMPLE 2 =====\n";
    Xplex::Model m;
    auto x1 = *m.newVariable("x1");
    auto x2 = *m.newVariable("x2");
    auto c1 = Xplex::Constraint("c1", 15);
    auto c2 = Xplex::Constraint("c2", 10);
    c1.setVariableCoefficient(x1, 3);
    c1.setVariableCoefficient(x2, 5);
    c2.setVariableCoefficient(x1, 5);
    c2.setVariableCoefficient(x2, 2);
    m.add(c1);
    m.add(c2);
    m.objective().setVariableCoefficient(x1, 5);
    m.objective().setVariableCoefficient(x2, 3);
    m.print();
    m.build();

    Xplex::Xplex xplex(&m);
    xplex.setVerbose(DEFAULT_VERBOSITY >= 1);
    xplex.solve();
    std::cout << xplex.getObjValue() << ": " << xplex.getValue(x1) << " " << xplex.getValue(x2) << "\n";

    std::cout << "\nExpected: x1=20/19  x2=45/19\n";
}

void example_3() { // Exemplo Duas Fases Livro p99
    std::cout << "\n\n\n===== EXAMPLE 3 =====\n";
    Xplex::Model m;
    auto x1 = *m.newVariable("x1");
    auto x2 = *m.newVariable("x2");
    auto c1 = Xplex::Constraint("c1", 21);
    auto c2 = Xplex::Constraint("c2", 13, Xplex::Constraint::InequalityType::GreaterOrEqual);
    auto c3 = Xplex::Constraint("c3", -1, Xplex::Constraint::InequalityType::Equal);
    c1.setVariableCoefficient(x1, 4);
    c1.setVariableCoefficient(x2, 1);
    c2.setVariableCoefficient(x1, 2);
    c2.setVariableCoefficient(x2, 3);
    c3.setVariableCoefficient(x1, 1);
    c3.setVariableCoefficient(x2, -1);
    m.add(c1);
    m.add(c2);
    m.add(c3);
    m.objective().setVariableCoefficient(x1, 6);
    m.objective().setVariableCoefficient(x2, -1);
    m.print();
    m.build();

    Xplex::Xplex xplex(&m);
    xplex.setVerbose(DEFAULT_VERBOSITY >= 1);
    xplex.solve();
    std::cout << xplex.getObjValue() << ": " << xplex.getValue(x1) << " " << xplex.getValue(x2) << "\n";

    std::cout << "\nExpected: x1=4  x2=5  x4=10\n";
}

void example_4() { // Exemplo Dual Livro p131
    std::cout << "\n\n\n===== EXAMPLE 4 =====\n";
    Xplex::Model m;
    auto x1 = *m.newVariable("x1");
    auto x2 = *m.newVariable("x2");
    auto x3 = *m.newVariable("x3");
    auto x4 = *m.newVariable("x4");
    auto x5 = *m.newVariable("x5");
    auto c1 = Xplex::Constraint("c1", 4, Xplex::Constraint::InequalityType::GreaterOrEqual);
    auto c2 = Xplex::Constraint("c2", 3, Xplex::Constraint::InequalityType::GreaterOrEqual);
    c1.setVariableCoefficient(x1, 1);
    c1.setVariableCoefficient(x2, 1);
    c1.setVariableCoefficient(x3, 2);
    c1.setVariableCoefficient(x4, 1);
    c1.setVariableCoefficient(x5, 3);
    c2.setVariableCoefficient(x1, 2);
    c2.setVariableCoefficient(x2, -2);
    c2.setVariableCoefficient(x3, 3);
    c2.setVariableCoefficient(x4, 1);
    c2.setVariableCoefficient(x5, 1);
    m.add(c1);
    m.add(c2);
    m.objective().multiplyBy(-1);
    m.objective().setVariableCoefficient(x1, 2);
    m.objective().setVariableCoefficient(x2, 3);
    m.objective().setVariableCoefficient(x3, 5);
    m.objective().setVariableCoefficient(x4, 2);
    m.objective().setVariableCoefficient(x5, 3);
    m.print();
    m.getDual().print();
    m.build();

    Xplex::Xplex xplex(&m);
    xplex.setVerbose(DEFAULT_VERBOSITY >= 1);
    xplex.solve();

    std::cout << "\nExpected: x1=1  x5=1\n";
}

void example_5_expressions();
void example_5() { // Exemplo Aula Anand - Análise de Sensibilidade
    std::cout << "\n\n\n===== EXAMPLE 5 =====\n";
    Xplex::Model m;
    auto x1 = *m.newVariable("x1");
    auto x2 = *m.newVariable("x2");
    auto c1 = Xplex::Constraint("c1", 4);
    auto c2 = Xplex::Constraint("c2", 12);
    auto c3 = Xplex::Constraint("c3", 18);
    c1.setVariableCoefficient(x1, 1);
    c2.setVariableCoefficient(x2, 2);
    c3.setVariableCoefficient(x1, 3);
    c3.setVariableCoefficient(x2, 2);
    m.add(c1);
    m.add(c2);
    m.add(c3);
    m.objective().setVariableCoefficient(x1, 3);
    m.objective().setVariableCoefficient(x2, 5);
    m.print();
    m.build();

    Xplex::Xplex xplex(&m);
    xplex.setVerbose(DEFAULT_VERBOSITY >= 1);
    xplex.solve();
    std::cout << xplex.getObjValue() << ": " << xplex.getValue(x1) << " " << xplex.getValue(x2) << "\n";

    std::cout << "\nExpected: x1=2  x2=6  x3=2\n";
}

void example_6_expressions();
void example_7_expressions();

int main(int argc, const char** argv) {
    std::cout << "XPLEX Revision " << GIT_COMMIT << "\n\nInvocation args:\n";

    std::vector<std::string> arg;
    for (int i = 0; i < argc; i++) {
        std::cout << i << ": " << argv[i] << "\n";
        arg.emplace_back(argv[i]);
    }
    std::cout << "\n";

    auto verbose_it = std::find(arg.begin(), arg.end(), "-v");
    if (verbose_it == arg.end()) DEFAULT_VERBOSITY = 0;
    else {
        arg.erase(verbose_it);
        DEFAULT_VERBOSITY = 0;
    }
    
    if (arg.size() == 2) {
    } else if (arg.size() == 1) {
        example_1();
        example_2();
        example_3();
        example_4();
        example_5();
        example_5_expressions();
        example_6_expressions();
        example_7_expressions();
    } else {
        throw std::runtime_error("Dont know what to do...");
    }
    return 0;
}

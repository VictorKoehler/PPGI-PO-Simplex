#include <iostream>
#include "Xplex.hpp"

static const bool verbose = false;

void example_1() {
    Xplex::Model m;
    auto x1 = m.newVariable("x1");
    auto x2 = m.newVariable("x2");
    auto x3 = m.newVariable("x3");
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
    m.objectiveFunction().setVariableCoefficient(x1, 1);
    m.objectiveFunction().setVariableCoefficient(x2, 2);
    m.objectiveFunction().setVariableCoefficient(x3, -1);
    m.build();

    Xplex::Xplex xplex(&m);
    xplex.setVerbose(verbose);
    xplex.solve();
    std::cout << xplex.getObjValue() << ": " << xplex.getValue(x1) << " " << xplex.getValue(x2) << " " << xplex.getValue(x3) << "\n";
}

void example_2() {
    Xplex::Model m;
    auto x1 = m.newVariable("x1");
    auto x2 = m.newVariable("x2");
    auto c1 = Xplex::Constraint("c1", 15);
    auto c2 = Xplex::Constraint("c2", 10);
    c1.setVariableCoefficient(x1, 3);
    c1.setVariableCoefficient(x2, 5);
    c2.setVariableCoefficient(x1, 5);
    c2.setVariableCoefficient(x2, 2);
    m.add(c1);
    m.add(c2);
    m.objectiveFunction().setVariableCoefficient(x1, 5);
    m.objectiveFunction().setVariableCoefficient(x2, 3);
    m.build();

    Xplex::Xplex xplex(&m);
    xplex.setVerbose(verbose);
    xplex.solve();
    std::cout << xplex.getObjValue() << ": " << xplex.getValue(x1) << " " << xplex.getValue(x2) << "\n";
}

void example_3() {
    Xplex::Model m;
    auto x1 = m.newVariable("x1");
    auto x2 = m.newVariable("x2");
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
    m.objectiveFunction().setVariableCoefficient(x1, 6);
    m.objectiveFunction().setVariableCoefficient(x2, -1);
    m.build();

    Xplex::Xplex xplex(&m);
    xplex.setVerbose(verbose);
    xplex.solve();
    std::cout << xplex.getObjValue() << ": " << xplex.getValue(x1) << " " << xplex.getValue(x2) << "\n";
}

int main() {
    // const Xplex::Model m;
    // Xplex::Xplex xplex(&m);
    // xplex.setVerbose(verbose);
    // xplex.solve();
    std::cout << "==EXAMPLE 1==\n";
    example_1();
    std::cout << "\n\n\n\n";
    std::cout << "==EXAMPLE 2==\n";
    example_2();
    std::cout << "\n\n\n\n";
    std::cout << "==EXAMPLE 3==\n";
    example_3();
}

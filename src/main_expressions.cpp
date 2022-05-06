#include <iostream>
#include "Xplex.hpp"

void example_5_expressions() { // Exemplo Aula Anand - Análise de Sensibilidade
    std::cout << "\n\n\n===== EXAMPLE 5 (Expressions) =====\n";
    Xplex::Model m;
    auto x1 = m.newVariable("x1");
    auto x2 = m.newVariable("x2");

    auto c1 =  1*x1        <= 4;
    auto c2 =         2*x2 <= 12;
    auto c3 =  3*x1 + 2*x2 <= 18;

    m.add(c1).add(c2).add(c3);
    m.objective() = Xplex::Maximize(3*x1 + 5*x2);
    m.build();

    Xplex::Xplex xplex(&m);
    xplex.setVerbose(DEFAULT_VERBOSITY >= 1);
    xplex.solve();
    std::cout << xplex.getObjValue() << ": " << xplex.getValue(x1) << " " << xplex.getValue(x2) << "\n";

    std::cout << "\nExpected: x1=2  x2=6  x3=2\n";
}

void example_6_expressions() { // Exemplo Dual Livro p39
    std::cout << "\n\n\n===== EXAMPLE 6 =====\n";
    Xplex::Model m;
    auto x1 = m.newVariable("x1");
    auto x2 = m.newVariable("x2");
    auto x3 = m.newVariable("x3", Xplex::Variable::Domain::NonPositive);
    auto x4 = m.newVariable("x4", Xplex::Variable::Domain::Unbounded);
    auto x5 = m.newVariable("x5");

    m.add_discard(  x1 + 2*x2 -   x3 +   x4 + 3*x5 >=  5);
    m.add_discard(4*x1        +   x3 - 2*x4 -   x5 <=  0);
    m.add_discard(            - 2*x3 +   x4 + 2*x5 >= -7);
    m.add_discard(3*x1 +   x2        -   x4 +   x5 ==  8);

    m.objective() = Xplex::Maximize(2*x1 + x2 -   x3 + 3*x4 -   x5);
    m.print();
    m.build();
    m.print();

    Xplex::Xplex xplex(&m);
    xplex.setVerbose(DEFAULT_VERBOSITY >= 1);
    xplex.solve();
}

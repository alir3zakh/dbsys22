#include "MyPlanEnumerator.hpp"


using namespace m;


template<typename PlanTable>
void MyPlanEnumerator::operator()(enumerate_tag, PlanTable &PT, const QueryGraph &G, const CostFunction &CF) const
{
    const AdjacencyMatrix &M = G.adjacency_matrix();
    auto &CE = Catalog::Get().get_database_in_use().cardinality_estimator();
    cnf::CNF condition; // Use this as join condition for PT.update(); we have fake cardinalities, so the condition
                        // doesn't matter.

    // TODO 3: Implement algorithm for plan enumeration (join ordering).
}

template void MyPlanEnumerator::operator()<PlanTableSmallOrDense&>(enumerate_tag, PlanTableSmallOrDense&, const QueryGraph&, const CostFunction&) const;
template void MyPlanEnumerator::operator()<PlanTableLargeAndSparse&>(enumerate_tag, PlanTableLargeAndSparse&, const QueryGraph&, const CostFunction&) const;

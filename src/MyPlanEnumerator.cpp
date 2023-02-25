#include "MyPlanEnumerator.hpp"

using namespace m;

template <typename PlanTable>
void MyPlanEnumerator::operator()(enumerate_tag, PlanTable &PT, const QueryGraph &G, const CostFunction &CF) const
{
    const AdjacencyMatrix &M = G.adjacency_matrix();
    auto &CE = Catalog::Get().get_database_in_use().cardinality_estimator();
    cnf::CNF condition; // Use this as join condition for PT.update(); we have fake cardinalities, so the condition
                        // doesn't matter.

    // TODO 3: Implement algorithm for plan enumeration (join ordering).
    auto superSet_size = PT.num_sources();

    const SmallBitset superSet((0b1 << superSet_size) - 1);

    M.for_each_CSG_pair_undirected(superSet,
                                   [&PT, &G, &CE, &CF, &condition](SmallBitset S1, SmallBitset S2)
                                   { PT.update(G, CE, CF, S1, S2, condition); });
}

template void MyPlanEnumerator::operator()<PlanTableSmallOrDense &>(enumerate_tag, PlanTableSmallOrDense &, const QueryGraph &, const CostFunction &) const;
template void MyPlanEnumerator::operator()<PlanTableLargeAndSparse &>(enumerate_tag, PlanTableLargeAndSparse &, const QueryGraph &, const CostFunction &) const;

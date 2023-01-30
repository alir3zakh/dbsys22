#include "MyPlanEnumerator.hpp"
#include <vector>

using namespace m;
using pair_type = std::pair<SmallBitset, SmallBitset>;

void get_subsets(const SmallBitset &superSet, SmallBitset &Left, size_t ind, const size_t &len,
                 std::vector<pair_type> &V, const AdjacencyMatrix &M)
{
    if (ind == len - 1)
    {
        auto Right = superSet - Left;
        if (Left.size() == 0 || Right.size() == 0 ||
            !M.is_connected(Left) || !M.is_connected(Right) || !M.is_connected(Left, Right))
            return;

        V.push_back(std::make_pair(Left, Right));
        return;
    }

    if (superSet[ind] == 0)
        return get_subsets(superSet, Left, ind + 1, len, V, M);

    get_subsets(superSet, Left, ind + 1, len, V, M);
    Left[ind] = 1;
    get_subsets(superSet, Left, ind + 1, len, V, M);
    Left[ind] = 0;
}

template <typename PlanTable>
void fill_PT(const Subproblem &S, const QueryGraph &G, PlanTable &PT,
             const CardinalityEstimator &CE, const CostFunction &CF, const cnf::CNF &condition,
             const AdjacencyMatrix &M, const size_t len)
{
    if (S.singleton() || PT.has_plan(S))
        return;

    SmallBitset temp(0);
    std::vector<pair_type> pairs;
    get_subsets(S, temp, 0, len, pairs, M);

    for (auto &e : pairs)
    {
        fill_PT(e.first, G, PT, CE, CF, condition, M, len);
        fill_PT(e.second, G, PT, CE, CF, condition, M, len);
        PT.update(G, CE, CF, e.first, e.second, condition);
    }
}

template <typename PlanTable>
void MyPlanEnumerator::operator()(enumerate_tag, PlanTable &PT, const QueryGraph &G, const CostFunction &CF) const
{
    const AdjacencyMatrix &M = G.adjacency_matrix();
    auto &CE = Catalog::Get().get_database_in_use().cardinality_estimator();
    cnf::CNF condition; // Use this as join condition for PT.update(); we have fake cardinalities, so the condition
                        // doesn't matter.

    // TODO 3: Implement algorithm for plan enumeration (join ordering).
    auto len = PT.num_sources();

    const SmallBitset superSet((0b1 << len) - 1);
    fill_PT(superSet, G, PT, CE, CF, condition, M, len);
}

template void MyPlanEnumerator::operator()<PlanTableSmallOrDense &>(enumerate_tag, PlanTableSmallOrDense &, const QueryGraph &, const CostFunction &) const;
template void MyPlanEnumerator::operator()<PlanTableLargeAndSparse &>(enumerate_tag, PlanTableLargeAndSparse &, const QueryGraph &, const CostFunction &) const;
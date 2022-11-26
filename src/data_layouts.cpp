#include "data_layouts.hpp"
#include <numeric>


using namespace m;
using namespace m::storage;


DataLayout MyNaiveRowLayoutFactory::make(std::vector<const Type*> types, std::size_t num_tuples) const
{
    // TODO 1.2: implement computing a row layout
    DataLayout DL;
    auto &row = DL.add_inode(1, 224 * 8);
    row.add_leaf(Type::Get_Integer(Type::TY_Vector, 4), 0, 0, 0);
    row.add_leaf(Type::Get_Char(Type::TY_Vector, 10), 1, 32, 0);
    row.add_leaf(Type::Get_Char(Type::TY_Vector, 32), 2, 128, 0);
    row.add_leaf(Type::Get_Char(Type::TY_Vector, 20), 3, 384, 0);
    row.add_leaf(Type::Get_Char(Type::TY_Vector, 80), 4, 544, 0);
    row.add_leaf(Type::Get_Char(Type::TY_Vector, 32), 5, 1184, 0);
    row.add_leaf(Type::Get_Integer(Type::TY_Vector, 8), 6, 1440, 0);
    row.add_leaf(Type::Get_Char(Type::TY_Vector, 32), 7, 1502, 0);
    row.add_leaf(Type::Get_Bitmap(Type::TY_Vector, 8), 8, 1760, 0);


    for(auto e: types) {
        std::cout << *e << ' ' << e->size() << ' ' << e->alignment() << ' ' << e->is_date() << std::endl;
    }

    return DL;
}

DataLayout MyOptimizedRowLayoutFactory::make(std::vector<const Type*> types, std::size_t num_tuples) const
{
    // TODO 1.3: implement computing an optimized row layout
    return DataLayout();
}

DataLayout MyPAX4kLayoutFactory::make(std::vector<const Type*> types, std::size_t num_tuples) const
{
    // TODO 1.4: implement computing a PAX layout
    return DataLayout();
}

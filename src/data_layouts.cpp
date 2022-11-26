#include "data_layouts.hpp"
#include <numeric>


using namespace m;
using namespace m::storage;


DataLayout MyNaiveRowLayoutFactory::make(std::vector<const Type*> types, std::size_t num_tuples) const
{
    // TODO 1.2: implement computing a row layout
    DataLayout DL;
    int INode_stride = 0;
    for (auto& type : types)
        INode_stride += ((type->size()-1) / 32 + 1) * 32;

    auto &row = DL.add_inode(1, INode_stride + 32);
    int cur_offset = 0, idx = 0;

    for (auto& type : types)
    {
        row.add_leaf(type, idx++, cur_offset, 0);
        cur_offset += ((type->size()-1) / 32 + 1) * 32;
    }
    
    row.add_leaf(Type::Get_Bitmap(Type::TY_Vector, idx), idx, cur_offset, 0);

    for(auto e: types) {
        std::cout << *e << ' ' << e->size() << ' ' << e->alignment() << ' ' << std::endl;
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

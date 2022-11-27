#include "data_layouts.hpp"
#include <numeric>


using namespace m;
using namespace m::storage;


DataLayout MyNaiveRowLayoutFactory::make(std::vector<const Type*> types, std::size_t num_tuples) const
{
    // TODO 1.2: implement computing a row layout
    DataLayout DL;

    // Computing inode stride
    int INode_stride = 0;
    uint64_t max_elem_alignemnt = 1;

    for (auto& type : types){
        // updating max alignemnt seen until now
        max_elem_alignemnt = std::max(max_elem_alignemnt, type->alignment());

        // extending inode stride by the amount of current type (leaf in feature)
        INode_stride += ((type->size()-1) / max_elem_alignemnt + 1) *                max_elem_alignemnt;
    }

    // Since minimum memory unit access is Byte, alignemnt could not be smaller
    max_elem_alignemnt = std::max(max_elem_alignemnt, uint64_t(8));

    // adding enough stride for NULL BITMAP
    if(types.back()->size() % max_elem_alignemnt == 0)
        INode_stride += max_elem_alignemnt;

    // IF the stride is not a multiply of alignment, rounding it up to next multiply of alignment
    if(INode_stride % max_elem_alignemnt)
        INode_stride = ((INode_stride + types.size()) / max_elem_alignemnt + 1) * max_elem_alignemnt;

    // Creading the inode
    auto &row = DL.add_inode(1, INode_stride);

    // Adding leafs to inode
    int cur_offset = 0, idx = 0;
    uint64_t max_elem_alignemnt_till_now = 1;

    for (auto& type : types){
        row.add_leaf(type, idx++, cur_offset, 0);

        // updating max alignemnt seen until now
        max_elem_alignemnt_till_now = std::max(max_elem_alignemnt_till_now, type->alignment());

        // computing offset for next leaf
        cur_offset += ((type->size()-1) / max_elem_alignemnt_till_now + 1) *                max_elem_alignemnt_till_now;
    }
    
    // Bitmap leaf
    row.add_leaf(Type::Get_Bitmap(Type::TY_Vector, idx), idx, cur_offset, 0);

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

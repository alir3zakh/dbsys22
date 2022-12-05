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

        // extending inode stride by the size + padding of current type (leaf in feature)
        if(INode_stride % type->alignment())
            INode_stride = (INode_stride / type->alignment() + 1) * type->alignment();
        INode_stride += type->size();
    }

    // Since minimum memory unit access is Byte, alignemnt could not be smaller
    max_elem_alignemnt = std::max(max_elem_alignemnt, uint64_t(8));

    // adding enough stride for NULL BITMAP
    INode_stride += types.size();

    // IF the stride is not a multiply of alignment, rounding it up to next multiply of max alignment
    if(INode_stride % max_elem_alignemnt)
        INode_stride = (INode_stride / max_elem_alignemnt + 1) * max_elem_alignemnt;

    // Creading the inode
    auto &row = DL.add_inode(1, INode_stride);

    // Adding leafs to inode
    int cur_offset = 0, idx = 0;

    for (auto& type : types){
        // beginning of current lead should by a multiply of the leaf's alignment
        if(cur_offset % type->alignment())
            cur_offset = (cur_offset / type->alignment() + 1) * type->alignment();

        row.add_leaf(type, idx++, cur_offset, 0);

        // computing offset for next leaf
        cur_offset += type->size();
    }
    
    // Bitmap leaf
    row.add_leaf(Type::Get_Bitmap(Type::TY_Vector, idx), idx, cur_offset, 0);

    return DL;
}

DataLayout MyOptimizedRowLayoutFactory::make(std::vector<const Type*> types, std::size_t num_tuples) const
{
    // TODO 1.3: implement computing an optimized row layout

    // storing initial indices
    std::vector<std::pair<const Type*, int>> types_indices;
    for (unsigned long ind = 0; ind < types.size(); ind++)
        types_indices.push_back(std::make_pair(types[ind], ind));

    // sorting types in descending order of their alignment
    std::sort(types_indices.begin(), types_indices.end(), [](auto a, auto b) {
        return a.first->alignment() > b.first->alignment();
    });
    DataLayout DL;

    // Computing inode stride
    int INode_stride = 0;
    uint64_t max_elem_alignemnt = types_indices.front().first->alignment();

    for (auto& type : types_indices){
        // extending inode stride by the size + padding of current type (leaf in feature)
        if(INode_stride % type.first->alignment())
            INode_stride = (INode_stride / type.first->alignment() + 1) * type.first->alignment();

        INode_stride += type.first->size();
    }

    // Since minimum memory unit access is Byte, alignemnt could not be smaller
    max_elem_alignemnt = std::max(max_elem_alignemnt, uint64_t(8));

    // adding enough stride for NULL BITMAP
    INode_stride += types.size();

    // IF the stride is not a multiply of alignment, rounding it up to next multiply of alignment
    if(INode_stride % max_elem_alignemnt)
        INode_stride = (INode_stride / max_elem_alignemnt + 1) * max_elem_alignemnt;

    // Creading the inode
    auto &row = DL.add_inode(1, INode_stride);

    
    // calculating offset of types before sorting them to initial order
    int cur_offset = 0;
    std::vector<int> offset(types.size());

    for (auto& type : types_indices){
        // beginning of current lead should by a multiply of the leaf's alignment
        if(cur_offset % type.first->alignment())
            cur_offset = (cur_offset / type.first->alignment() + 1) * type.first->alignment();

        offset[type.second] = cur_offset;

        // computing offset for next leaf
        cur_offset += type.first->size();
    }

    //sorting types to initial order
    std::sort(types_indices.begin(), types_indices.end(), [](auto a, auto b) {
        return a.second < b.second;
    });


    // Adding leafs to inode
    for (auto& type : types_indices)
        row.add_leaf(type.first, type.second, offset[type.second], 0);
    
    // Bitmap leaf
    row.add_leaf(Type::Get_Bitmap(Type::TY_Vector, types.size()), types.size(), cur_offset, 0);

    return DL;
}

DataLayout MyPAX4kLayoutFactory::make(std::vector<const Type*> types, std::size_t num_tuples) const
{
    // TODO 1.4: implement computing a PAX layout
    // storing initial indices
    std::vector<std::pair<const Type*, int>> types_indices;
    for (unsigned long ind = 0; ind < types.size(); ind++)
        types_indices.push_back(std::make_pair(types[ind], ind));

    // sorting types in descending order of their alignment
    std::sort(types_indices.begin(), types_indices.end(), [](auto a, auto b) {
        return a.first->alignment() > b.first->alignment();
    });
    DataLayout DL;

    // Computing inode stride
    int INode_stride = 4096 * 8, row_length = 0;
    uint64_t max_elem_alignemnt = types_indices.front().first->alignment();

    for (auto& type : types_indices){
        // extending inode stride by the size + padding of current type (leaf in feature)
        if(row_length % type.first->alignment())
            row_length = (INode_stride / type.first->alignment() + 1) * type.first->alignment();

        row_length += type.first->size();
    }

    // Since minimum memory unit access is Byte, alignemnt could not be smaller
    max_elem_alignemnt = std::max(max_elem_alignemnt, uint64_t(8));

    // adding enough stride for NULL BITMAP
    row_length += types.size();

    // Creading the inode
    auto &row = DL.add_inode(INode_stride / row_length, INode_stride);

    
    // calculating offset of types before sorting them to initial order
    int cur_offset = 0;
    std::vector<int> offset(types.size());

    for (auto& type : types_indices){
        offset[type.second] = cur_offset;

        // computing offset for next leaf
        cur_offset += type.first->size() * (INode_stride / row_length);
    }

    //sorting types to initial order
    std::sort(types_indices.begin(), types_indices.end(), [](auto a, auto b) {
        return a.second < b.second;
    });


    // Adding leafs to inode
    for (auto& type : types_indices)
        row.add_leaf(type.first, type.second, offset[type.second], type.first->size());
    
    // Bitmap leaf
    row.add_leaf(Type::Get_Bitmap(Type::TY_Vector, types.size()), types.size(), cur_offset, 1);

    return DL;
}

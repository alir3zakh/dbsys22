#pragma once

#include "mutable/util/macro.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <concepts>
#include <cstdint>
#include <vector>


/** Require that \tparam T is an *orderable* type, i.e. that two instances of \tparam T can be compared less than and
 * equals. */
template<typename T>
concept orderable = requires (T a, T b) {
    { a <  b } -> std::same_as<bool>;
    { a == b } -> std::same_as<bool>;
};

/** Require that \tparam T is sortable, i.e. that it is `orderable`, movable, and swappable. */
template<typename T>
concept sortable = std::movable<T> and std::swappable<T> and orderable<T>;

/** A replacement for `std::pair`, that does not leak/enforce a particular data layout. */
template<typename First, typename Second>
struct ref_pair
{
    private:
    std::reference_wrapper<First> first_;
    std::reference_wrapper<Second> second_;

    public:
    ref_pair(First &first, Second &second) : first_(first), second_(second) { }

    First & first() const { return first_.get(); }
    Second & second() const { return second_.get(); }
};


/** Implements a B+-tree of \tparam Key - \tparam Value pairs.  The exact size of a tree node is given as \tparam
 * NodeSizeInBytes and the exact node alignment is given as \tparam NodeAlignmentInBytes.  The implementation must
 * guarantee that nodes are properly allocated to satisfy the alignment. */
template<
    typename Key,
    std::movable Value,
    std::size_t NodeSizeInBytes,
    std::size_t NodeAlignmentInBytes = NodeSizeInBytes
>
requires sortable<Key> and std::copyable<Key>
struct BTree
{
    using key_type = Key;
    using mapped_type = Value;
    using size_type = std::size_t;
    using pair_type = ref_pair<Key, Value>;

    ///> the size of BTree nodes (both `INode` and `Leaf`)
    static constexpr size_type NODE_SIZE_IN_BYTES = NodeSizeInBytes;
    ///> the aignment of BTree nodes (both `INode` and `Leaf`)
    static constexpr size_type NODE_ALIGNMENT_IN_BYTES = NodeAlignmentInBytes;

    private:
    /** Computes the number of key-value pairs per `Leaf`, considering the specified `NodeSizeInBytes`. */
    static constexpr size_type compute_num_keys_per_leaf()
    {
        /* TODO 1.2.1 */
        size_type pair_size = sizeof(Key) + sizeof(Value);
        size_type useable = NodeSizeInBytes 
                            - 2 * sizeof(nullptr) 
                            - sizeof(std::vector<Key>)
                            - sizeof(std::vector<Value>);

        return NodeSizeInBytes / pair_size;
    };

    /** Computes the number of keys per `INode`, considering the specified `NodeSizeInBytes`. */
    static constexpr size_type compute_num_keys_per_inode()
    {
        /* TODO 1.3.1 */
        return 0;
    };

    public:
    ///> the number of key-value pairs per `Leaf`
    static constexpr size_type NUM_KEYS_PER_LEAF = compute_num_keys_per_leaf();
    ///> the number of keys per `INode`
    static constexpr size_type NUM_KEYS_PER_INODE = compute_num_keys_per_inode();

    /** This class implements leaves of the B+-tree. */
    struct alignas(NODE_ALIGNMENT_IN_BYTES) Leaf
    {
        /* TODO 1.2.2 define fields */
        std::vector<Key> keys;
        std::vector<Value> data;
        Leaf* next = nullptr;
        Leaf* prev = nullptr;

        
        /* TODO 1.2.3 define methods */
        template<typename It>
        Leaf(It begin, It end)
        {
            keys.reserve(NUM_KEYS_PER_LEAF);
            data.reserve(NUM_KEYS_PER_LEAF);


            while (begin < end){
                keys.push_back((*begin).first);
                data.push_back((*begin).second);
                begin++;
            }
        }
    };
    static_assert(sizeof(Leaf) <= NODE_SIZE_IN_BYTES, "Leaf exceeds its size limit");

    /** This class implements inner nodes of the B+-tree. */
    struct alignas(NODE_ALIGNMENT_IN_BYTES) INode
    {
        /* TODO 1.3.2 define fields */

        /* TODO 1.3.3 define methods */
        INode();

        template <typename It>
        INode* insert(It begin, It end);
    };
    static_assert(sizeof(INode) <= NODE_SIZE_IN_BYTES, "INode exceeds its size limit");

    private:
    template<bool IsConst>
    struct the_iterator
    {
        friend struct BTree;

        static constexpr bool is_const = IsConst;
        using value_type = std::conditional_t<is_const, const mapped_type, mapped_type>;

        private:
        using leaf_type = std::conditional_t<is_const, const Leaf, Leaf>;

        /* TODO 1.4.3 define fields */
        Leaf* current = nullptr;
        size_t index = 0;

        public:

        the_iterator() = default;

        the_iterator(Leaf * leafptr, size_t ind){
            this->current = leafptr;
            this->index = ind;
        }

        the_iterator(the_iterator<false> other)
        requires is_const
        {
            /* TODO 1.4.3 */
            this->current = other->current;
            this->index = other->index;
        }

        bool operator==(the_iterator other) const {
            /* TODO 1.4.3 */
            return (this->current == other.current && this->index == other.index);
        }
        bool operator!=(the_iterator other) const { return not operator==(other); }

        the_iterator & operator++() {
            /* TODO 1.4.3 */
            if(this->current != nullptr){
                index++;
                if(index == current->keys.size()){
                    current = current->next;
                    index = 0;
                }
            }

            return *this;
        }

        the_iterator operator++(int) {
            the_iterator copy(this->current, this->index);
            operator++();
            return copy;
        }

        ref_pair<const key_type, value_type> operator*() const {
            /* TODO 1.4.3 */
            const key_type k = current->keys[index];
            value_type val = current->data[index];

            ref_pair<const key_type, value_type> pair(k, val);
            std::cout << pair.second() << pair.second() << std::endl;
            

            return ref_pair(k, val);
        }
    };

    template<bool IsConst>
    struct the_range
    {
        static constexpr bool is_const = IsConst;
        using iter_t = the_iterator<is_const>;
        private:
        iter_t begin_, end_;

        public:
        the_range(iter_t begin, iter_t end) : begin_(begin), end_(end) { }

        bool empty() const { return begin() == end(); }

        iter_t begin() const { return begin_; }
        iter_t end() const { return end_; }
    };

    public:
    using iterator = the_iterator<false>;
    using const_iterator = the_iterator<true>;

    using range = the_range<false>;
    using const_range = the_range<true>;

    private:
    /* TODO 1.4.1 define fields */
    size_type tree_size = 0;
    size_type tree_height = 0;


    iterator begin_iter = iterator();
    iterator end_iter = iterator();

    const_iterator const_begin_iter = const_iterator();
    const_iterator const_end_iter = const_iterator();

    std::vector<Leaf*> leaves;


    public:
    /** Bulkloads the data in the range from `begin` (inclusive) to `end` (exclusive) into a fresh `BTree` and returns
     * it. */
    template<typename It>
    static BTree Bulkload(It begin, It end)
    requires requires (It it) {
        key_type(std::move(it->first));
        mapped_type(std::move(it->second));
    }
    {
        /* TODO 1.4.4 */
        BTree<Key, Value, NodeSizeInBytes> tree(begin, end);
        return tree;
    }

    ~BTree() {
        // std::cout << "Destroyed!\n";
        for (size_t i = 0; i < leaves.size(); i++)
        {
            delete leaves[i];
        }
        
    }
    private:
    BTree() = default;

    
    template<typename it>
    BTree(it begin, it end) : tree_size(end-begin)
    {
        // NUM_LEAFS = tree_size / NUM_KEYS_PER_LEAF;
        // if(tree_size % NUM_KEYS_PER_LEAF) NUM_LEAFS++;
        // std::ios::sync_with_stdio(false);

        std::cout << "Constructor!\n";    


        while((end - begin) > NUM_KEYS_PER_LEAF){
            leaves.push_back(new Leaf(begin, begin+NUM_KEYS_PER_LEAF));
            begin += NUM_KEYS_PER_LEAF;
        }

        std::cout << "NUM_KEYS_PER_LEAF: " << NUM_KEYS_PER_LEAF << std::endl;
        std::cout << "Pair left: " << (end - begin) << std::endl;

        if(end - begin > 0)
            leaves.push_back(new Leaf(begin, end));

        if(leaves.size() > 0){
            std::cout << "Num Leaves: " << leaves.size() << std::endl;
            begin_iter = iterator(leaves[0], 0);
            const_begin_iter = const_iterator(leaves[0], 0);


            for (size_t i = 1; i < leaves.size(); i++)
            {
                Leaf* prev_ptr = leaves[i-1];
                Leaf* current_ptr = leaves[i];

                prev_ptr->next = current_ptr;
                current_ptr->prev = prev_ptr;
            }
            
        }
    }


    public:
    ///> returns the size of the tree, i.e. the number of key-value pairs
    size_type size() const { /* TODO 1.4.2 */ return this->tree_size; }
    ///> returns the number if inner/non-leaf levels, a.k.a. the height
    size_type height() const { /* TODO 1.4.2 */ return this->tree_height; }

    /** Returns an `iterator` to the smallest key-value pair of the tree, if any, and `end()` otherwise. */
    iterator begin() { /* TODO 1.4.3 */ return begin_iter; }
    /** Returns the past-the-end `iterator`. */
    iterator end() { /* TODO 1.4.3 */ return end_iter; }
    /** Returns an `const_iterator` to the smallest key-value pair of the tree, if any, and `end()` otherwise. */
    const_iterator begin() const { /* TODO 1.4.3 */ return const_begin_iter; }
    /** Returns the past-the-end `iterator`. */
    const_iterator end() const { /* TODO 1.4.3 */ return const_end_iter; }
    /** Returns an `const_iterator` to the smallest key-value pair of the tree, if any, and `end()` otherwise. */
    const_iterator cbegin() const { return begin(); }
    /** Returns the past-the-end `iterator`. */
    const_iterator cend() const { return end(); }

    /** Returns a `const_iterator` to the first element with the given \p key, if any, and `end()` otherwise. */
    const_iterator find(const key_type &key) const {
        /* TODO 1.4.5 */
        M_unreachable("not implemented");
    }
    /** Returns an `iterator` to the first element with the given \p key, if any, and `end()` otherwise. */
    iterator find(const key_type &key) {
        /* TODO 1.4.5 */
        M_unreachable("not implemented");
    }

    /** Returns a `const_range` of all elements with key in the interval `[lo, hi)`, i.e. `lo` including and `hi`
     * excluding. */
    const_range find_range(const key_type &lo, const key_type &hi) const {
        /* TODO 1.4.6 */
        M_unreachable("not implemented");
    }
    /** Returns a `range` of all elements with key in the interval `[lo, hi)`, i.e. `lo` including and `hi` excluding.
     * */
    range find_range(const key_type &lo, const key_type &hi) {
        /* TODO 1.4.6 */
        M_unreachable("not implemented");
    }

    /** Returns a `const_range` of all elements with key equals to \p key. */
    const_range equal_range(const key_type &key) const {
        /* TODO 1.4.7 */
        M_unreachable("not implemented");
    }
    /** Returns a `range` of all elements with key equals to \p key. */
    range equal_range(const key_type &key) {
        /* TODO 1.4.7 */
        M_unreachable("not implemented");
    }
};

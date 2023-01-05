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
template <typename T>
concept orderable = requires(T a, T b) {
                        {
                            a < b
                            } -> std::same_as<bool>;
                        {
                            a == b
                            } -> std::same_as<bool>;
                    };

/** Require that \tparam T is sortable, i.e. that it is `orderable`, movable, and swappable. */
template <typename T>
concept sortable = std::movable<T> and std::swappable<T> and orderable<T>;

/** A replacement for `std::pair`, that does not leak/enforce a particular data layout. */
template <typename First, typename Second>
struct ref_pair
{
private:
    std::reference_wrapper<First> first_;
    std::reference_wrapper<Second> second_;

public:
    ref_pair(First &first, Second &second) : first_(first), second_(second) {}

    First &first() const { return first_.get(); }
    Second &second() const { return second_.get(); }
};

/** Implements a B+-tree of \tparam Key - \tparam Value pairs.  The exact size of a tree node is given as \tparam
 * NodeSizeInBytes and the exact node alignment is given as \tparam NodeAlignmentInBytes.  The implementation must
 * guarantee that nodes are properly allocated to satisfy the alignment. */
template <
    typename Key,
    std::movable Value,
    std::size_t NodeSizeInBytes,
    std::size_t NodeAlignmentInBytes = NodeSizeInBytes>
    requires sortable<Key> and std::copyable<Key>
struct BTree
{
    using key_type = Key;
    using mapped_type = Value;
    using size_type = std::size_t;
    using pair_type = ref_pair<const key_type, const mapped_type>;

    ///> the size of BTree nodes (both `INode` and `Leaf`)
    static constexpr size_type NODE_SIZE_IN_BYTES = NodeSizeInBytes;
    ///> the aignment of BTree nodes (both `INode` and `Leaf`)
    static constexpr size_type NODE_ALIGNMENT_IN_BYTES = NodeAlignmentInBytes;

private:
    /** Computes the number of key-value pairs per `Leaf`, considering the specified `NodeSizeInBytes`. */
    static constexpr size_type compute_num_keys_per_leaf()
    {
        /* TODO 1.2.1 */
        size_type pair_size = sizeof(pair_type);
        size_type usable = NodeSizeInBytes - sizeof(nullptr) - sizeof(std::vector<pair_type>);

        return NodeSizeInBytes / pair_size;
    };

    /** Computes the number of keys per `INode`, considering the specified `NodeSizeInBytes`. */
    static constexpr size_type compute_num_keys_per_inode()
    {
        /* TODO 1.3.1 */
        size_type pair_size = sizeof(pair_type);
        size_type usable = NodeSizeInBytes - sizeof(void *);

        return NodeSizeInBytes / pair_size;
    };

public:
    ///> the number of key-value pairs per `Leaf`
    static constexpr size_type NUM_KEYS_PER_LEAF = compute_num_keys_per_leaf();
    ///> the number of keys per `INode`
    static constexpr size_type NUM_KEYS_PER_INODE = compute_num_keys_per_inode();
    static constexpr size_type NUM_PIVOTS_PER_INODE = NUM_KEYS_PER_INODE + 1;

    /** This class implements leaves of the B+-tree. */
    struct alignas(NODE_ALIGNMENT_IN_BYTES) Leaf
    {
        /* TODO 1.2.2 define fields */
        std::vector<pair_type> pairs;
        Leaf *next = nullptr;

        /* TODO 1.2.3 define methods */
        template <typename It>
        Leaf(It begin, It end)
        {
            pairs.reserve(NUM_KEYS_PER_LEAF);
            for (auto iter = begin; iter != end; iter++)
                pairs.push_back(ref_pair((*iter).first, (*iter).second));
        }

        key_type get_pivot()
        {
            return pairs.back().first();
        }
    };
    static_assert(sizeof(Leaf) <= NODE_SIZE_IN_BYTES, "Leaf exceeds its size limit");

    /** This class implements inner nodes of the B+-tree. */
    struct alignas(NODE_ALIGNMENT_IN_BYTES) INode
    {
        /* TODO 1.3.2 define fields */
        std::vector<key_type> keys;
        std::vector<void *> node_ptrs;

        /* TODO 1.3.3 define methods */
        template <typename It>
        INode(It begin, It end)
        {
            keys.reserve(NUM_KEYS_PER_INODE);
            node_ptrs.reserve(NUM_PIVOTS_PER_INODE);

            for (auto iter = begin; iter < end; iter++)
            {
                node_ptrs.push_back(*iter);
                if (keys.size() < NUM_KEYS_PER_INODE)
                    keys.push_back((*iter)->get_pivot());
            }
        }

        key_type get_pivot()
        {
            return keys.back();
        }
    };
    static_assert(sizeof(INode) <= NODE_SIZE_IN_BYTES, "INode exceeds its size limit");

private:
    template <bool IsConst>
    struct the_iterator
    {
        friend struct BTree;

        static constexpr bool is_const = IsConst;
        using value_type = std::conditional_t<is_const, const mapped_type, mapped_type>;

    private:
        using leaf_type = std::conditional_t<is_const, const Leaf, Leaf>;

        /* TODO 1.4.3 define fields */
        leaf_type *current = nullptr;
        size_t index = 0;

    public:
        the_iterator(){};

        the_iterator(Leaf *leafptr, size_t ind = 0)
        {
            current = leafptr;
            index = ind;
        }

        the_iterator(the_iterator<false> other)
            requires is_const
        {
            /* TODO 1.4.3 */
            current = other.current;
            index = other.index;
        }

        bool operator==(the_iterator other) const
        {
            /* TODO 1.4.3 */
            return (current == other.current && index == other.index);
        }
        bool operator!=(the_iterator other) const { return not operator==(other); }

        the_iterator &operator++()
        {
            /* TODO 1.4.3 */
            if (this->current != nullptr)
            {
                index++;
                if (index == current->pairs.size())
                {
                    current = current->next;
                    index = 0;
                }
            }

            return *this;
        }

        the_iterator operator++(int)
        {
            the_iterator copy(this);
            operator++();
            return copy;
        }

        ref_pair<const key_type, const value_type> operator*() const
        {
            /* TODO 1.4.3 */
            return current->pairs[index];
        }
    };

    template <bool IsConst>
    struct the_range
    {
        static constexpr bool is_const = IsConst;
        using iter_t = the_iterator<is_const>;

    private:
        iter_t begin_, end_;

    public:
        the_range(iter_t begin, iter_t end) : begin_(begin), end_(end) {}

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

    std::vector<Leaf *> leaves;
    std::vector<std::vector<INode *>> nodes;
    void *root = nullptr;

public:
    /** Bulkloads the data in the range from `begin` (inclusive) to `end` (exclusive) into a fresh `BTree` and returns
     * it. */
    template <typename It>
    static BTree Bulkload(It begin, It end)
        requires requires(It it) {
                     key_type(std::move(it->first));
                     mapped_type(std::move(it->second));
                 }
    {
        /* TODO 1.4.4 */
        BTree<Key, Value, NodeSizeInBytes> tree(begin, end);
        return tree;
    }

    ~BTree()
    {
        // std::cout << "Destroyed!\n";
        for (auto &ptr : leaves)
            delete ptr;
    }

private:
    BTree() = default;

    template <typename it>
    BTree(it begin, it end) : tree_size(end - begin)
    {
        while ((end - begin) > NUM_KEYS_PER_LEAF)
        {
            leaves.push_back(new Leaf(begin, begin + NUM_KEYS_PER_LEAF));
            begin += NUM_KEYS_PER_LEAF;
        }
        if (end - begin > 0)
            leaves.push_back(new Leaf(begin, end));

        if (leaves.size() > 0)
        {
            root = build_tree();

            std::cout << "TREE_SIZE: " << tree_size << std::endl;
            std::cout << "NUM_KEYS_PER_LEAF: " << NUM_KEYS_PER_LEAF << std::endl;
            std::cout << "NUM LEAVES: " << leaves.size() << std::endl;
            std::cout << "NUM_KEYS_PER_INODE " << NUM_KEYS_PER_INODE << std::endl;

            for (auto &level : nodes)
                std::cout << "NUM INOES: " << level.size() << std::endl;
            std::cout << "----------------------\n";
        }
    }

    void *build_tree()
    {
        begin_iter = iterator(leaves[0]);
        const_begin_iter = const_iterator(leaves[0]);

        for (size_t i = 1; i < leaves.size(); i++)
            leaves[i - 1]->next = leaves[i];

        if (leaves.size() == 1)
            return leaves.back();

        nodes.push_back(std::vector<INode *>());

        auto begin = leaves.begin();
        auto end = leaves.end();
        while ((end - begin) > NUM_PIVOTS_PER_INODE)
        {
            nodes.back().push_back(new INode(begin, begin + NUM_PIVOTS_PER_INODE));
            begin += NUM_PIVOTS_PER_INODE;
        }
        if (end - begin > 0)
        {
            nodes.back().push_back(new INode(begin, end));
        }

        while (nodes.back().size() != 1)
        {
            auto begin = nodes.back().begin();
            auto end = nodes.back().end();

            nodes.push_back(std::vector<INode *>());

            while ((end - begin) > NUM_PIVOTS_PER_INODE)
            {
                nodes.back().push_back(new INode(begin, begin + NUM_PIVOTS_PER_INODE));
                begin += NUM_PIVOTS_PER_INODE;
            }
            if (end - begin > 0)
            {
                nodes.back().push_back(new INode(begin, end));
            }
        }

        return nodes.back().back();
    }

public:
    ///> returns the size of the tree, i.e. the number of key-value pairs
    size_type size() const
    { /* TODO 1.4.2 */
        return this->tree_size;
    }
    ///> returns the number if inner/non-leaf levels, a.k.a. the height
    size_type height() const
    { /* TODO 1.4.2 */
        return this->tree_height;
    }

    /** Returns an `iterator` to the smallest key-value pair of the tree, if any, and `end()` otherwise. */
    iterator begin()
    { /* TODO 1.4.3 */
        return begin_iter;
    }
    /** Returns the past-the-end `iterator`. */
    iterator end()
    { /* TODO 1.4.3 */
        return end_iter;
    }
    /** Returns an `const_iterator` to the smallest key-value pair of the tree, if any, and `end()` otherwise. */
    const_iterator begin() const
    { /* TODO 1.4.3 */
        return const_begin_iter;
    }
    /** Returns the past-the-end `iterator`. */
    const_iterator end() const
    { /* TODO 1.4.3 */
        return const_end_iter;
    }
    /** Returns an `const_iterator` to the smallest key-value pair of the tree, if any, and `end()` otherwise. */
    const_iterator cbegin() const { return begin(); }
    /** Returns the past-the-end `iterator`. */
    const_iterator cend() const { return end(); }

    /** Returns a `const_iterator` to the first element with the given \p key, if any, and `end()` otherwise. */
    const_iterator find(const key_type &key) const
    {
        /* TODO 1.4.5 */
        M_unreachable("not implemented");
    }
    /** Returns an `iterator` to the first element with the given \p key, if any, and `end()` otherwise. */
    iterator find(const key_type &key)
    {
        /* TODO 1.4.5 */
        M_unreachable("not implemented");
    }

    /** Returns a `const_range` of all elements with key in the interval `[lo, hi)`, i.e. `lo` including and `hi`
     * excluding. */
    const_range find_range(const key_type &lo, const key_type &hi) const
    {
        /* TODO 1.4.6 */
        M_unreachable("not implemented");
    }
    /** Returns a `range` of all elements with key in the interval `[lo, hi)`, i.e. `lo` including and `hi` excluding.
     * */
    range find_range(const key_type &lo, const key_type &hi)
    {
        /* TODO 1.4.6 */
        M_unreachable("not implemented");
    }

    /** Returns a `const_range` of all elements with key equals to \p key. */
    const_range equal_range(const key_type &key) const
    {
        /* TODO 1.4.7 */
        M_unreachable("not implemented");
    }
    /** Returns a `range` of all elements with key equals to \p key. */
    range equal_range(const key_type &key)
    {
        /* TODO 1.4.7 */
        M_unreachable("not implemented");
    }
};

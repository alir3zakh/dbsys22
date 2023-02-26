#pragma once

#include "mutable/util/macro.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <concepts>
#include <cstdint>
#include <vector>

/** Require that \tparam T is an *orderable* type, i.e. that two instances of \tparam T can be compare_key_paird less than and
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
    // using pair_type = ref_pair<const key_type, const mapped_type>;

    ///> the size of BTree nodes (both `INode` and `Leaf`)
    static constexpr size_type NODE_SIZE_IN_BYTES = NodeSizeInBytes;
    ///> the aignment of BTree nodes (both `INode` and `Leaf`)
    static constexpr size_type NODE_ALIGNMENT_IN_BYTES = NodeAlignmentInBytes;

private:
    /** Computes the number of key-value pairs per `Leaf`, considering the specified `NodeSizeInBytes`. */
    static constexpr size_type compute_num_keys_per_leaf()
    {
        /* TODO 1.2.1 */
        size_type pair_size = sizeof(key_type) + sizeof(mapped_type);
        size_type usable = (NodeSizeInBytes - sizeof(size_t) - 2 * sizeof(Leaf *)) / pair_size;

        return usable - 1;
    };

    /** Computes the number of keys per `INode`, considering the specified `NodeSizeInBytes`. */
    static constexpr size_type compute_num_keys_per_inode()
    {
        /* TODO 1.3.1 */
        size_type pair_size = sizeof(key_type) + sizeof(Node_Entity *);
        size_type usable = (NodeSizeInBytes - sizeof(size_t) - sizeof(BTree *)) / pair_size;

        return usable - 1;
    };

public:
    ///> the number of key-value pairs per `Leaf`
    static constexpr size_type NUM_KEYS_PER_LEAF = compute_num_keys_per_leaf();
    ///> the number of keys per `INode`
    static constexpr size_type NUM_KEYS_PER_INODE = compute_num_keys_per_inode();

    struct Node_Entity
    {
        virtual key_type get_pivot() = 0;
        virtual void find(const key_type &key) = 0;
        virtual void lower_bound(const key_type &key) = 0;
        virtual void upper_bound(const key_type &key) = 0;
        virtual ~Node_Entity() = default;
    };

    /** This class implements leaves of the B+-tree. */
    struct alignas(NODE_ALIGNMENT_IN_BYTES) Leaf : public Node_Entity
    {
        /* TODO 1.2.2 define fields */
        std::array<key_type, NUM_KEYS_PER_LEAF> keys;
        std::array<mapped_type, NUM_KEYS_PER_LEAF> vals;
        size_t length = 0;
        Leaf *next = nullptr;
        BTree *tree;

        /* TODO 1.2.3 define methods */
        template <typename It>
        Leaf(const It &begin, const It &end, BTree *Tree) : tree(Tree)
        {
            for (auto iter = begin; iter != end; iter++, length++)
            {
                keys[length] = (*iter).first;
                vals[length] = (*iter).second;
            }
        }

        key_type get_pivot() override { return keys[length - 1]; }

        void find(const key_type &key) override
        {
            if (std::binary_search(keys.begin(), keys.begin() + length, key))
            {
                auto it = std::lower_bound(keys.begin(), keys.begin() + length, key);
                tree->find_iter = iterator(this, it - keys.begin());
                return;
            }

            tree->find_iter = iterator(this, -1);
        }

        void lower_bound(const key_type &key) override
        {
            auto it = std::lower_bound(keys.begin(), keys.begin() + length, key);

            if (it == keys.begin() + length)
                tree->lower_bound_iter = iterator(this, -1);
            else
                tree->lower_bound_iter = iterator(this, it - keys.begin());
        }

        void upper_bound(const key_type &key) override
        {
            auto it = std::upper_bound(keys.begin(), keys.begin() + length, key);

            if (it == keys.begin() + length)
                tree->upper_bound_iter = iterator(this, -1);

            else
                tree->upper_bound_iter = iterator(this, it - keys.begin());
        }
    };
    static_assert(sizeof(Leaf) <= NODE_SIZE_IN_BYTES, "Leaf exceeds its size limit");

    /** This class implements inner nodes of the B+-tree. */
    struct alignas(NODE_ALIGNMENT_IN_BYTES) INode : public Node_Entity
    {
        /* TODO 1.3.2 define fields */
        std::array<key_type, NUM_KEYS_PER_INODE> keys;
        std::array<Node_Entity *, NUM_KEYS_PER_INODE> node_ptrs;
        size_t length = 0;
        BTree *tree;

        /* TODO 1.3.3 define methods */
        template <typename It>
        INode(const It &begin, const It &end, BTree *Tree) : tree(Tree)
        {
            for (auto iter = begin; iter < end; iter++, length++)
            {
                node_ptrs[length] = *iter;
                keys[length] = (*iter)->get_pivot();
            }
        }

        key_type get_pivot() override { return keys[length - 1]; }

        void find(const key_type &key) override
        {
            auto it = std::lower_bound(keys.begin(), keys.begin() + length, key);
            if (it == keys.begin() + length)
                tree->find_iter = iterator(nullptr, -1);
            else
                node_ptrs[it - keys.begin()]->find(key);
        }

        void lower_bound(const key_type &key) override
        {
            auto it = std::lower_bound(keys.begin(), keys.begin() + length, key);
            if (it == keys.begin() + length)
                tree->lower_bound_iter = iterator(nullptr, -1);
            else
                node_ptrs[it - keys.begin()]->lower_bound(key);
        }

        void upper_bound(const key_type &key) override
        {
            auto it = std::upper_bound(keys.begin(), keys.begin() + length, key);
            if (it == keys.begin() + length)
                tree->upper_bound_iter = iterator(nullptr, -1);
            else
                node_ptrs[it - keys.begin()]->upper_bound(key);
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
        int index = 0;

    public:
        the_iterator(){};

        the_iterator(Leaf *leafptr, int ind = 0) : current(leafptr), index(ind) {}

        the_iterator(the_iterator<false> other) : current(other.current), index(other.index) {}

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
                if (index == current->length)
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

        ref_pair<const key_type, value_type> operator*() const
        {
            /* TODO 1.4.3 */
            return ref_pair<const key_type, value_type>(current->keys[index], current->vals[index]);
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
    iterator find_iter = iterator();
    iterator lower_bound_iter = iterator();
    iterator upper_bound_iter = iterator();

    const_iterator const_begin_iter = const_iterator();
    const_iterator const_end_iter = const_iterator();

    Node_Entity *root = nullptr;
    std::vector<std::vector<Node_Entity *>> nodes;

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
        for (auto &level : nodes)
            for (auto &node : level)
                delete node;
    }

private:
    BTree() = default;

    template <typename it>
    BTree(it begin, it end) : tree_size(end - begin)
    {
        nodes.push_back(std::vector<Node_Entity *>());
        while ((end - begin) > NUM_KEYS_PER_LEAF)
        {
            nodes.back().push_back(new Leaf(begin, begin + NUM_KEYS_PER_LEAF, this));
            begin += NUM_KEYS_PER_LEAF;
        }
        if (end - begin > 0)
            nodes.back().push_back(new Leaf(begin, end, this));

        if (nodes.back().size() > 0)
            root = build_tree();
    }

    Node_Entity *build_tree()
    {
        std::vector<Node_Entity *> &leaves = nodes[0];

        begin_iter = iterator(static_cast<Leaf *>(leaves[0]));
        const_begin_iter = const_iterator(static_cast<Leaf *>(leaves[0]));

        for (size_t i = 1; i < leaves.size(); i++)
        {
            Leaf *prev_leaf = static_cast<Leaf *>(leaves[i - 1]);
            Leaf *curr_leaf = static_cast<Leaf *>(leaves[i]);
            prev_leaf->next = curr_leaf;
        }

        while (nodes.back().size() != 1)
        {
            auto begin = nodes.back().begin();
            auto end = nodes.back().end();
            nodes.push_back(std::vector<Node_Entity *>());

            while ((end - begin) > NUM_KEYS_PER_INODE)
            {
                nodes.back().push_back(new INode(begin, begin + NUM_KEYS_PER_INODE, this));
                begin += NUM_KEYS_PER_INODE;
            }
            if (end - begin > 0)
                nodes.back().push_back(new INode(begin, end, this));
        }

        return nodes.back().back();
    }

public:
    ///> returns the size of the tree, i.e. the number of key-value pairs
    size_type size() const { return tree_size; }
    ///> returns the number if inner/non-leaf levels, a.k.a. the height
    size_type height() const { return tree_height; }

    /** Returns an `iterator` to the smallest key-value pair of the tree, if any, and `end()` otherwise. */
    iterator begin() { return begin_iter; }
    /** Returns the past-the-end `iterator`. */
    iterator end() { return end_iter; }
    /** Returns an `const_iterator` to the smallest key-value pair of the tree, if any, and `end()` otherwise. */
    const_iterator begin() const { return const_begin_iter; }
    /** Returns the past-the-end `iterator`. */
    const_iterator end() const { return const_end_iter; }
    /** Returns an `const_iterator` to the smallest key-value pair of the tree, if any, and `end()` otherwise. */
    const_iterator cbegin() const { return begin(); }
    /** Returns the past-the-end `iterator`. */
    const_iterator cend() const { return end(); }

    /** Returns a `const_iterator` to the first element with the given \p key, if any, and `end()` otherwise. */
    const_iterator find(const key_type &key) const
    {
        /* TODO 1.4.5 */
        if (root == nullptr)
            return end();

        root->find(key);
        if (find_iter.index == -1)
            return end();

        return find_iter;
    }
    /** Returns an `iterator` to the first element with the given \p key, if any, and `end()` otherwise. */
    iterator find(const key_type &key)
    {
        /* TODO 1.4.5 */
        if (root == nullptr)
            return end();

        root->find(key);
        if (find_iter.index == -1)
            return end();

        return find_iter;
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
        if (root == nullptr)
            return range(end(), end());

        root->lower_bound(lo);
        auto lo_iter = lower_bound_iter;

        if (lo_iter.index == -1)
            return range(end(), end());

        root->lower_bound(hi);
        auto hi_iter = lower_bound_iter;
        // while ((*hi_iter).first() < hi)
        // {
        //     ++hi_iter;
        //     if (hi_iter == end())
        //         break;
        // }

        if (hi_iter.index == -1)
            hi_iter = end();

        return range(lo_iter, hi_iter);
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
        if (root == nullptr)
            return range(end(), end());

        root->lower_bound(key);
        if (lower_bound_iter.index == -1)
            return range(end(), end());

        root->upper_bound(key);

        if (upper_bound_iter.index == -1)
            return range(lower_bound_iter, end());

        return range(lower_bound_iter, upper_bound_iter);
    }
};

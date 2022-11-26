#include <catch2/catch.hpp>

#include "data_layouts.hpp"


using namespace m;
using namespace m::storage;


TEST_CASE("NaiveRowLayout", "[milestone1]")
{
    Catalog::Clear(); // drop all data
    auto &C = m::Catalog::Get();

    try {
        C.register_data_layout("row_naive", std::make_unique<MyNaiveRowLayoutFactory>(), "row layout (naïve)");
    } catch (std::invalid_argument) { }

    auto &DB = C.add_database(C.pool("test_db"));
    auto &table = DB.add_table(C.pool("test"));

    SECTION("INT(4)")
    {
        /* Fill table with attributes. */
        table.push_back(C.pool("a"), m::Type::Get_Integer(m::Type::TY_Vector, 4));

        /* Create store and data layout. */
        table.store(C.create_store(table));
        table.layout(C.data_layout("row_naive"));
        const auto &layout = table.layout();

        /* Root must be an indefinite sequence of rows. */
        CHECK(not layout.is_finite());

        /* Check stride of row. */
        CHECK(layout.stride_in_bits() == 64);

        auto &child_node = layout.child();

        /* Check that the child INode models a single row. */
        CHECK(child_node.num_tuples() == 1);

        /* Check that child node is an INode. */
        auto inode = cast<const DataLayout::INode>(&child_node);
        REQUIRE(inode);

        /* Validate the INode. */
        CHECK(inode->num_children() == 2);
        CHECK(inode->at(0).offset_in_bits == 0);
        CHECK(inode->at(0).stride_in_bits == 0);
        CHECK(inode->at(1).offset_in_bits == 32);
        CHECK(inode->at(1).stride_in_bits == 0);

        /* Validate the data leaf. */
        auto attr_leaf = cast<const DataLayout::Leaf>(inode->at(0).ptr.get());
        REQUIRE(attr_leaf);
        CHECK(attr_leaf->num_tuples() == 1);
        CHECK(attr_leaf->index() == 0);
        CHECK(attr_leaf->type()->is_integral());
        CHECK(attr_leaf->type()->size() == 32);

        /* Validate the NULL bitmap. */
        auto null_bitmap = cast<const DataLayout::Leaf>(inode->at(1).ptr.get());
        REQUIRE(null_bitmap);
        CHECK(null_bitmap->num_tuples() == 1);
        CHECK(null_bitmap->index() == 1);
        CHECK(null_bitmap->type()->is_bitmap());
        CHECK(null_bitmap->type()->size() == 1);
    }

    SECTION("DOUBLE")
    {
        /* Fill table with attributes. */
        table.push_back(C.pool("a"), m::Type::Get_Double(m::Type::TY_Vector));

        /* Create store and data layout. */
        table.store(C.create_store(table));
        table.layout(C.data_layout("row_naive"));
        const auto &layout = table.layout();

        /* Root must be an indefinite sequence of rows. */
        CHECK(not layout.is_finite());

        /* Check stride of row. */
        CHECK(layout.stride_in_bits() == 128);

        auto &child_node = layout.child();

        /* Check that the child INode models a single row. */
        CHECK(child_node.num_tuples() == 1);

        /* Check that child node is an INode. */
        auto inode = cast<const DataLayout::INode>(&child_node);
        REQUIRE(inode);

        /* Validate the INode. */
        CHECK(inode->num_children() == 2);
        CHECK(inode->at(0).offset_in_bits == 0);
        CHECK(inode->at(0).stride_in_bits == 0);
        CHECK(inode->at(1).offset_in_bits == 64);
        CHECK(inode->at(1).stride_in_bits == 0);

        /* Validate the data leaf. */
        auto attr_leaf = cast<const DataLayout::Leaf>(inode->at(0).ptr.get());
        REQUIRE(attr_leaf);
        CHECK(attr_leaf->num_tuples() == 1);
        CHECK(attr_leaf->index() == 0);
        CHECK(attr_leaf->type()->is_double());
        CHECK(attr_leaf->type()->size() == 64);

        /* Validate the NULL bitmap. */
        auto null_bitmap = cast<const DataLayout::Leaf>(inode->at(1).ptr.get());
        REQUIRE(null_bitmap);
        CHECK(null_bitmap->num_tuples() == 1);
        CHECK(null_bitmap->index() == 1);
        CHECK(null_bitmap->type()->is_bitmap());
        CHECK(null_bitmap->type()->size() == 1);
    }

    SECTION("INT(2)")
    {
        /* Fill table with attributes. */
        table.push_back(C.pool("a"), m::Type::Get_Integer(m::Type::TY_Vector, 2));

        /* Create store and data layout. */
        table.store(C.create_store(table));
        table.layout(C.data_layout("row_naive"));
        const auto &layout = table.layout();

        /* Root must be an indefinite sequence of rows. */
        CHECK(not layout.is_finite());

        /* Check stride of row. */
        CHECK(layout.stride_in_bits() == 32);

        auto &child_node = layout.child();

        /* Check that the child INode models a single row. */
        CHECK(child_node.num_tuples() == 1);

        /* Check that child node is an INode. */
        auto inode = cast<const DataLayout::INode>(&child_node);
        REQUIRE(inode);

        /* Validate the INode. */
        CHECK(inode->num_children() == 2);
        CHECK(inode->at(0).offset_in_bits == 0);
        CHECK(inode->at(0).stride_in_bits == 0);
        CHECK(inode->at(1).offset_in_bits == 16);
        CHECK(inode->at(1).stride_in_bits == 0);

        /* Validate the data leaf. */
        auto attr_leaf = cast<const DataLayout::Leaf>(inode->at(0).ptr.get());
        REQUIRE(attr_leaf);
        CHECK(attr_leaf->num_tuples() == 1);
        CHECK(attr_leaf->index() == 0);
        CHECK(attr_leaf->type()->is_integral());
        CHECK(attr_leaf->type()->size() == 16);

        /* Validate the NULL bitmap. */
        auto null_bitmap = cast<const DataLayout::Leaf>(inode->at(1).ptr.get());
        REQUIRE(null_bitmap);
        CHECK(null_bitmap->num_tuples() == 1);
        CHECK(null_bitmap->index() == 1);
        CHECK(null_bitmap->type()->is_bitmap());
        CHECK(null_bitmap->type()->size() == 1);
    }

    SECTION("CHAR(3)")
    {
        /* Fill table with attributes. */
        table.push_back(C.pool("a"), m::Type::Get_Char(m::Type::TY_Vector, 3));

        /* Create store and data layout. */
        table.store(C.create_store(table));
        table.layout(C.data_layout("row_naive"));
        const auto &layout = table.layout();

        /* Root must be an indefinite sequence of rows. */
        CHECK(not layout.is_finite());

        /* Check stride of row. */
        CHECK(layout.stride_in_bits() == 32);

        auto &child_node = layout.child();

        /* Check that the child INode models a single row. */
        CHECK(child_node.num_tuples() == 1);

        /* Check that child node is an INode. */
        auto inode = cast<const DataLayout::INode>(&child_node);
        REQUIRE(inode);

        /* Validate the INode. */
        CHECK(inode->num_children() == 2);
        CHECK(inode->at(0).offset_in_bits == 0);
        CHECK(inode->at(0).stride_in_bits == 0);
        CHECK(inode->at(1).offset_in_bits == 24);
        CHECK(inode->at(1).stride_in_bits == 0);

        /* Validate the data leaf. */
        auto attr_leaf = cast<const DataLayout::Leaf>(inode->at(0).ptr.get());
        REQUIRE(attr_leaf);
        CHECK(attr_leaf->num_tuples() == 1);
        CHECK(attr_leaf->index() == 0);
        CHECK(attr_leaf->type()->is_character_sequence());
        CHECK(attr_leaf->type()->size() == 24);

        /* Validate the NULL bitmap. */
        auto null_bitmap = cast<const DataLayout::Leaf>(inode->at(1).ptr.get());
        REQUIRE(null_bitmap);
        CHECK(null_bitmap->num_tuples() == 1);
        CHECK(null_bitmap->index() == 1);
        CHECK(null_bitmap->type()->is_bitmap());
        CHECK(null_bitmap->type()->size() == 1);
    }

    SECTION("BOOL")
    {
        /* Fill table with attributes. */
        table.push_back(C.pool("a"), m::Type::Get_Boolean(m::Type::TY_Vector));

        /* Create store and data layout. */
        table.store(C.create_store(table));
        table.layout(C.data_layout("row_naive"));
        const auto &layout = table.layout();

        /* Root must be an indefinite sequence of rows. */
        CHECK(not layout.is_finite());

        /* Check stride of row. */
        CHECK(layout.stride_in_bits() == 8);

        auto &child_node = layout.child();

        /* Check that the child INode models a single row. */
        CHECK(child_node.num_tuples() == 1);

        /* Check that child node is an INode. */
        auto inode = cast<const DataLayout::INode>(&child_node);
        REQUIRE(inode);

        /* Validate the INode. */
        CHECK(inode->num_children() == 2);
        CHECK(inode->at(0).offset_in_bits == 0);
        CHECK(inode->at(0).stride_in_bits == 0);
        CHECK(inode->at(1).offset_in_bits == 1);
        CHECK(inode->at(1).stride_in_bits == 0);

        /* Validate the data leaf. */
        auto attr_leaf = cast<const DataLayout::Leaf>(inode->at(0).ptr.get());
        REQUIRE(attr_leaf);
        CHECK(attr_leaf->num_tuples() == 1);
        CHECK(attr_leaf->index() == 0);
        CHECK(attr_leaf->type()->is_boolean());
        CHECK(attr_leaf->type()->size() == 1);

        /* Validate the NULL bitmap. */
        auto null_bitmap = cast<const DataLayout::Leaf>(inode->at(1).ptr.get());
        REQUIRE(null_bitmap);
        CHECK(null_bitmap->num_tuples() == 1);
        CHECK(null_bitmap->index() == 1);
        CHECK(null_bitmap->type()->is_bitmap());
        CHECK(null_bitmap->type()->size() == 1);
    }

    SECTION("five booleans")
    {
        /* Fill table with attributes. */
        table.push_back(C.pool("a"), m::Type::Get_Boolean(m::Type::TY_Vector));
        table.push_back(C.pool("b"), m::Type::Get_Boolean(m::Type::TY_Vector));
        table.push_back(C.pool("c"), m::Type::Get_Boolean(m::Type::TY_Vector));
        table.push_back(C.pool("d"), m::Type::Get_Boolean(m::Type::TY_Vector));
        table.push_back(C.pool("e"), m::Type::Get_Boolean(m::Type::TY_Vector));

        /* Create store and data layout. */
        table.store(C.create_store(table));
        table.layout(C.data_layout("row_naive"));
        const auto &layout = table.layout();

        /* Root must be an indefinite sequence of rows. */
        CHECK(not layout.is_finite());

        /* Check stride of row. */
        CHECK(layout.stride_in_bits() == 16);

        auto &child_node = layout.child();

        /* Check that the child INode models a single row. */
        CHECK(child_node.num_tuples() == 1);

        /* Check that child node is an INode. */
        auto inode = cast<const DataLayout::INode>(&child_node);
        REQUIRE(inode);

        /* Validate the INode. */
        CHECK(inode->num_children() == 6);
        CHECK(inode->at(0).offset_in_bits == 0);
        CHECK(inode->at(0).stride_in_bits == 0);
        CHECK(inode->at(1).offset_in_bits == 1);
        CHECK(inode->at(1).stride_in_bits == 0);
        CHECK(inode->at(2).offset_in_bits == 2);
        CHECK(inode->at(2).stride_in_bits == 0);
        CHECK(inode->at(3).offset_in_bits == 3);
        CHECK(inode->at(3).stride_in_bits == 0);
        CHECK(inode->at(4).offset_in_bits == 4);
        CHECK(inode->at(4).stride_in_bits == 0);
        CHECK(inode->at(5).offset_in_bits == 5);
        CHECK(inode->at(5).stride_in_bits == 0);

        /* Validate the first boolean. */
        auto b0 = cast<const DataLayout::Leaf>(inode->at(0).ptr.get());
        REQUIRE(b0);
        CHECK(b0->num_tuples() == 1);
        CHECK(b0->index() == 0);
        CHECK(b0->type()->is_boolean());
        CHECK(b0->type()->size() == 1);

        /* Validate the first boolean. */
        auto b1 = cast<const DataLayout::Leaf>(inode->at(1).ptr.get());
        REQUIRE(b1);
        CHECK(b1->num_tuples() == 1);
        CHECK(b1->index() == 1);
        CHECK(b1->type()->is_boolean());
        CHECK(b1->type()->size() == 1);

        /* Validate the second boolean. */
        auto b2 = cast<const DataLayout::Leaf>(inode->at(2).ptr.get());
        REQUIRE(b2);
        CHECK(b2->num_tuples() == 1);
        CHECK(b2->index() == 2);
        CHECK(b2->type()->is_boolean());
        CHECK(b2->type()->size() == 1);

        /* Validate the third boolean. */
        auto b3 = cast<const DataLayout::Leaf>(inode->at(3).ptr.get());
        REQUIRE(b3);
        CHECK(b3->num_tuples() == 1);
        CHECK(b3->index() == 3);
        CHECK(b3->type()->is_boolean());
        CHECK(b3->type()->size() == 1);

        /* Validate the fourth boolean. */
        auto b4 = cast<const DataLayout::Leaf>(inode->at(4).ptr.get());
        REQUIRE(b4);
        CHECK(b4->num_tuples() == 1);
        CHECK(b4->index() == 4);
        CHECK(b4->type()->is_boolean());
        CHECK(b4->type()->size() == 1);

        /* Validate the NULL bitmap. */
        auto null_bitmap = cast<const DataLayout::Leaf>(inode->at(5).ptr.get());
        REQUIRE(null_bitmap);
        CHECK(null_bitmap->num_tuples() == 1);
        CHECK(null_bitmap->index() == 5);
        CHECK(null_bitmap->type()->is_bitmap());
        CHECK(null_bitmap->type()->size() == 5);
    }

    SECTION("simple table")
    {
        /* Fill table with attributes. */
        table.push_back(C.pool("id"), m::Type::Get_Integer(m::Type::TY_Vector, 4));
        table.push_back(C.pool("name"), m::Type::Get_Char(m::Type::TY_Vector, 20));
        table.push_back(C.pool("cakeday"), m::Type::Get_Date(m::Type::TY_Vector));
        table.push_back(C.pool("in_assessment"), m::Type::Get_Boolean(m::Type::TY_Vector));
        table.push_back(C.pool("salary"), m::Type::Get_Double(m::Type::TY_Vector));

        /* Create store and data layout. */
        table.store(C.create_store(table));
        table.layout(C.data_layout("row_naive"));
        const auto &layout = table.layout();

        /* Root must be an indefinite sequence of rows. */
        CHECK(not layout.is_finite());

        /* Check stride of row. */
        CHECK(layout.stride_in_bits() == 384);

        auto &child_node = layout.child();

        /* Check that the child INode models a single row. */
        CHECK(child_node.num_tuples() == 1);

        /* Check that child node is an INode. */
        auto inode = cast<const DataLayout::INode>(&child_node);
        REQUIRE(inode);

        /* Validate the INode. */
        CHECK(inode->num_children() == 6);
        /* id */
        CHECK(inode->at(0).offset_in_bits == 0);
        CHECK(inode->at(0).stride_in_bits == 0);
        /* name */
        CHECK(inode->at(1).offset_in_bits == 32);
        CHECK(inode->at(1).stride_in_bits == 0);
        /* cakeday */
        CHECK(inode->at(2).offset_in_bits == 192);
        CHECK(inode->at(2).stride_in_bits == 0);
        /* in_assessment */
        CHECK(inode->at(3).offset_in_bits == 224);
        CHECK(inode->at(3).stride_in_bits == 0);
        /* salary */
        CHECK(inode->at(4).offset_in_bits == 256);
        CHECK(inode->at(4).stride_in_bits == 0);
        /* NULL bitmap */
        CHECK(inode->at(5).offset_in_bits == 320);
        CHECK(inode->at(5).stride_in_bits == 0);

        /* Validate `id`. */
        auto attr_id = cast<const DataLayout::Leaf>(inode->at(0).ptr.get());
        REQUIRE(attr_id);
        CHECK(attr_id->num_tuples() == 1);
        CHECK(attr_id->index() == 0);
        CHECK(attr_id->type()->is_integral());
        CHECK(attr_id->type()->size() == 32);

        /* Validate `name`. */
        auto attr_name = cast<const DataLayout::Leaf>(inode->at(1).ptr.get());
        REQUIRE(attr_name);
        CHECK(attr_name->num_tuples() == 1);
        CHECK(attr_name->index() == 1);
        CHECK(attr_name->type()->is_character_sequence());
        CHECK(attr_name->type()->size() == 160);

        /* Validate `cakeday`. */
        auto attr_cakeday = cast<const DataLayout::Leaf>(inode->at(2).ptr.get());
        REQUIRE(attr_cakeday);
        CHECK(attr_cakeday->num_tuples() == 1);
        CHECK(attr_cakeday->index() == 2);
        CHECK(attr_cakeday->type()->is_date());
        CHECK(attr_cakeday->type()->size() == 32);

        /* Validate `in_assessment`. */
        auto attr_in_assessment = cast<const DataLayout::Leaf>(inode->at(3).ptr.get());
        REQUIRE(attr_in_assessment);
        CHECK(attr_in_assessment->num_tuples() == 1);
        CHECK(attr_in_assessment->index() == 3);
        CHECK(attr_in_assessment->type()->is_boolean());
        CHECK(attr_in_assessment->type()->size() == 1);

        /* Validate `salary`. */
        auto attr_salary = cast<const DataLayout::Leaf>(inode->at(4).ptr.get());
        REQUIRE(attr_salary);
        CHECK(attr_salary->num_tuples() == 1);
        CHECK(attr_salary->index() == 4);
        CHECK(attr_salary->type()->is_double());
        CHECK(attr_salary->type()->size() == 64);

        /* Validate the NULL bitmap. */
        auto null_bitmap = cast<const DataLayout::Leaf>(inode->at(5).ptr.get());
        REQUIRE(null_bitmap);
        CHECK(null_bitmap->num_tuples() == 1);
        CHECK(null_bitmap->index() == 5);
        CHECK(null_bitmap->type()->is_bitmap());
        CHECK(null_bitmap->type()->size() == 5);
    }
}

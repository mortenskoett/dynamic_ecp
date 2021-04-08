#include <eCP/index/shared/data_structure.hpp>
#include <iostream>
#include <vector>

/*
 * Point data type
 */
Point::Point(const float* descriptor_, unsigned id_)
    : descriptor(new float[globals::g_vector_dimensions])
    , id(id_)
{
  std::copy(descriptor_, descriptor_ + globals::g_vector_dimensions, descriptor);
}

Point::Point(const std::vector<float> descriptor_, unsigned id_)
    : descriptor(new float[globals::g_vector_dimensions])
    , id(id_)
{
  std::copy(descriptor_.begin(), descriptor_.end(), descriptor);
}

Point::~Point() { delete[] descriptor; }

// Copy constructor.
Point::Point(const Point& other)
    : Point(other.descriptor, other.id)
{
}

// Move constructor.
Point::Point(Point&& other) noexcept
    : descriptor(nullptr)
    , id(0)
{
  swap(*this, other);
}

// Copy+Move assignment operator. Notice takes concrete instance.
Point& Point::operator=(Point other) noexcept
{
  swap(*this, other);
  return *this;
}

void swap(Point& fst, Point& snd)
{
  using std::swap;
  swap(fst.id, snd.id);
  swap(fst.descriptor, snd.descriptor);
}

/*
 * Node data type
 */
Node::Node() {}

Node::Node(Point p)
    : points{std::move(p)}
{
}

Point* Node::get_leader() { return &points[0]; }

/*
 * Index data type
 */
Index::Index()
    : L(0)
    , sc(0)
    , sn(0)
    , size(0)
    , root(Node{})
{
}

// Used to construct the index incrementally.
Index::Index(unsigned sc_, unsigned sn_)
    : L(1)
    , sc(sc_)
    , sn(sn_)
    , size(0)
    , root(Node{})
{
}

// Used to batch construct the index.
Index::Index(unsigned L_, unsigned index_size, unsigned sc_, unsigned sn_, Node root_node)
    : L(L_)
    , sc(sc_)
    , sn(sn_)
    , size(index_size)
    , root(root_node)
{
}

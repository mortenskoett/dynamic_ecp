#include <eCP/index/shared/data_structure.hpp>
#include <iostream>
#include <vector>

/*
 * Point data type
 */
Point::Point(const float* descriptor_, unsigned long id_)
    : descriptor(new float[globals::g_vector_dimensions])
    , id(id_)
{
  std::copy(descriptor_, descriptor_ + globals::g_vector_dimensions, descriptor);
}

Point::Point(const std::vector<float> descriptor_, unsigned long id_)
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
Point& Point::operator=(Point other) noexcept {
  swap(*this, other);
  return *this;
}

void swap(Point& fst, Point& snd) {
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
 * ReclusteringScheme
 */
ReclusteringScheme::ReclusteringScheme(unsigned sc_, unsigned hi_mark_, ReclusteringPolicy cluster_policy_,
                                       ReclusteringPolicy node_policy_)
    : lo_mark(sc_)
    , hi_mark(hi_mark_)
    , cluster_policy(cluster_policy_)
    , node_policy(node_policy_)
{
}

ReclusteringScheme::ReclusteringScheme()
    : lo_mark(100)
    , hi_mark(100 * (1 + 0.3))
    , cluster_policy(ReclusteringPolicy::AVERAGE)
    , node_policy(ReclusteringPolicy::ABSOLUTE)
{
}

/*
 * Index data type
 */
Index::Index()
    : L(0)
    , size(0)
    , scheme(ReclusteringScheme{})
    , root(Node{})
{
}

Index::Index(unsigned L_, unsigned long index_size, Node root_node, ReclusteringScheme scheme_)
    : L(L_)
    , size(index_size)
    , scheme(scheme_)
    , root(root_node)
{
}

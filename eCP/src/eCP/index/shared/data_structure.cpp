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
Index::Index(unsigned sc_, ReclusteringPolicy cluster_policy_, ReclusteringPolicy node_policy_,
             Node root_node)
    : L(1)
    , sc(sc_)
    , sn(sc_)
    , size(1)
    , cluster_policy(cluster_policy_)
    , node_policy(node_policy_)
    , root(root_node)
{
  root.children.reserve(sn);
}

// Used to batch construct the index.
Index::Index(unsigned L_, unsigned index_size, unsigned sc_, unsigned sn_, ReclusteringPolicy cluster_policy_,
             ReclusteringPolicy node_policy_, Node root_node)
    : L(L_)
    , sc(sc_)
    , sn(sn_)
    , size(index_size)
    , cluster_policy(cluster_policy_)
    , node_policy(node_policy_)
    , root(root_node)
{
}

// WIP
// bool AbsoluteClusterPolicy::is_reclustering_necessary(Node* const cluster)
//{
//  return (cluster->points.size() > threshold);
//}

// bool AverageClusterPolicy::is_reclustering_necessary(Node* const parent)
//{
//      DescendantData descriptor_data = {count_descriptors(parent)};
//      auto all_parent_descriptors{descriptor_data.amount};
//      auto max_points_in_single_cluster{descriptor_data.max_value};

//      float average_number_per_cluster =
//          all_parent_descriptors / static_cast<float>(parent->children.size());  // Cast for float arithm.

//      if (average_number_per_cluster > threshold) {
//        return true;
//      }

//      if (max_points_in_single_cluster > max_size) {
//        return true;
//      }
//}

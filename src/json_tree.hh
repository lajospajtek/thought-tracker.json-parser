#ifndef __JSON_HH__
#define __JSON_HH__

#include <string>
#include <stack>
#include <vector>
#include <iostream>

namespace json {

/**
 * \brief A DOM tree node. The root of the inheritence hierarchy.
 */
class node {
public:
	//! \brief The DOM tree node destructor. Overwritten in subclasses.
	virtual ~node() {}
	/**
	 * \brief Abstract method that prints the DOM tree node to an output stream.
	 * 
	 * \param os The output stream
	 * \return The output stream 
	 */
	virtual std::ostream& print(std::ostream&) const = 0;
};

/**
 * \brief A leaf node in the DOM tree corresponding to a value of string type.
 */
class string_node : public node {
public:
	/**
	 * \brief The string node constructor.
	 * 
	 * \param n The string
	 */
	string_node(const std::string& n) : name_(n) {}
	/**
	 * \brief Accessor method. Gets the string.
	 * 
	 * \return The string
	 */
	const std::string& name() const { return name_; }
	/**
	 * \brief Prints the string to the output stream.
	 * 
	 * \param os The output stream the string is printed to.
	 * \return The output stream the string is printed to.
	 */
	std::ostream& print(std::ostream& os) const;
private:
	//! \brief The string.
	const std::string name_;
};

/**
 * \brief A leaf node in the DOM tree corresponding to a value of integer or floating point type.
 */
class number_node : public node {
public:
	/**
	 * \brief The number node constructor.
	 * 
	 * \param no The number
	 */
	number_node(double no) : number_(no) {}
	/**
	 * \brief Accessor method. Gets the number.
	 * 
	 * \return The number
	 */
	double value() const { return number_; }
	/**
	 * \brief Prints the number to the output stream.
	 * 
	 * \param os The output stream the number is printed to.
	 * \return The output stream the number is printed to.
	 */
	std::ostream& print(std::ostream& os) const;
private:
	//! \brief The number.
	const double number_;
};

/**
 * \brief A leaf node in the DOM tree corresponding to a boolean constant.
 */
class bool_node : public node {
public:
	/**
	 * \brief The boolean node constructor.
	 * 
	 * \param n The boolean constant
	 */
	bool_node(bool n) : flag_(n) {}
	/**
	 * \brief Accessor method. Gets the boolean constant.
	 */
	bool value() const { return flag_; }
	/**
	 * \brief Prints the boolean to the output stream.
	 * 
	 * \param os The output stream the boolean is printed to.
	 * \return The output stream the boolean is printed to.
	 */
	std::ostream& print(std::ostream& os) const;
private:
	//! \brief The boolean constant
	const bool flag_;
};

/**
 * \brief A non-leaf node in the DOM tree
 */
class internal_node : public node {
};

/**
 * \brief An object or an array node in the DOM tree
 */
class coll_node : public internal_node {
public:
	/**
	 * \brief Adds a key:value pair node to the object node or
	 * an array element to the array node.
	 */
	virtual void add(const node *) = 0;
};

/**
 * \brief The DOM tree node like a key:value pair but without key.
 * Works like a sort of wrapper around another DOM tree node.
 */
class root_node : public coll_node {
public:
	//! \brief The constructor
	root_node() : value_(0) {}
	//! \brief Destructs the DOM tree node that it wraps around.
	virtual ~root_node();
	/**
	 * \brief Adds the DOM tree node this node wraps around.
	 * 
	 * \param v The DOM tree node to add
	 */
	virtual void add(const node *v) { value_ = v; }
	/**
	 * \brief Accessor method. Gets the wrapped object.
	 * 
	 * \return The DOM tree node that this node wraps around.
	 */
	const node *value() const { return value_; }
	/**
	 * \brief Prints the wrapped object to the output stream.
	 * 
	 * \param os The output stream the wrapped object is printed to.
	 * \return The output stream the wrapped object is printed to.
	 */
	virtual std::ostream& print(std::ostream& os) const;
protected:
	//! \brief The wrapped object.
	const node *value_;
};

/**
 * \brief A key:value. The value part is inherited from json::root_node.
 */
class obj_node : public root_node {
public:
	/**
	 * \brief The constructor of the key:value pair.
	 * 
	 * \param n The key.
	 */
	obj_node(const std::string& n) : name_(n) {}
	/**
	 * \brief Accessor method. Gets the key.
	 * 
	 * \return The key.
	 */
	const std::string& name() const { return name_; }
	/**
	 * \brief Associates a string value to the key.
	 * 
	 * \param n The string value.
	 */
	void add(const std::string& n) { root_node::add(new string_node(n)); }
	/**
	 * \brief Associates a floating point value to the key.
	 * 
	 * \param no The floating point value.
	 */
	void add(double no) { root_node::add(new number_node(no)); }
	/**
	 * \brief Associates a boolean value to the key.
	 * 
	 * \param f The boolean value.
	 */
	void add(bool f) { root_node::add(new bool_node(f)); }
	/**
	 * \brief Associates a structured value (object or array) to the key.
	 * 
	 * \param v The structured value.
	 */
	void add(const node *v) { root_node::add(v); }
	/**
	 * \brief Prints the key:value pair to the output stream.
	 * 
	 * \param os The output stream the key:value pair is printed to.
	 * \return The output stream the key:value pair is printed to.
	 */
	std::ostream& print(std::ostream&) const;
private:
	//! \brief The key.
	const std::string name_;
};

/**
 * \brief A DOM tree node corresponding to an array.
 */
class array_node : public coll_node {
public:
	//! \brief Destructs the array element nodes as well.
	~array_node();
	/**
	 * \brief Adds a structured element (an object or an array) to the array.
	 * 
	 * \param n The structured value to add/
	 */
	void add(const node *n) { v.push_back(n); }
	/**
	 * \brief Adds a string element to the array.
	 * 
	 * \param n The string element to add to the array.
	 */
	void add(const std::string& n) { add(new string_node(n)); }
	/**
	 * \brief Adds a floating point element to the array.
	 * 
	 * \param no The floating point element to add to the array.
	 */
	void add(double no) { add(new number_node(no)); }
	/**
	 * \brief Adds a boolean element to the array.
	 * 
	 * \param f The boolean element to add to the array.
	 */
	void add(bool f) { add(new bool_node(f)); }
	/**
	 * \brief Prints the array to the output stream.
	 * 
	 * \param os The output stream the array is printed to.
	 * \return The output stream the array is printed to.
	 */
	std::ostream& print(std::ostream&) const;
private:
	//! \brief The array elements.
	std::vector<const node *> v;
};

/**
 * \brief A DOM tree node corresponding to an object.
 */
class obj_list_node : public internal_node {
public:
	//! \brief Destructs the list of key:value pairs as well.
	~obj_list_node();
	/**
	 * \brief Adds a key:value pair to the object.
	 * 
	 * \param obj The key:value pair to add to the object. 
	 */
	void add(const obj_node *obj) { v.push_back(obj); }
	/**
	 * \brief Prints the object to the output stream.
	 * 
	 * \param os The output stream the object is printed to.
	 * \return The output stream the object is printed to.
	 */
	std::ostream& print(std::ostream&) const;
private:
	//! \brief The list of key:value pairs.
	std::vector<const obj_node *> v;
};

}

/**
 * \brief Operator printing the DOM tree rooted in r to the output stream os
 * 
 * \param os The output stream the DOM tree is printed to
 * \param r The DOM tree root node.
 * \return The output stream the DOM tree is printed to
 */
extern std::ostream& operator<<(std::ostream&, const json::root_node&);

/**
 * \brief Creates a \link json::obj_list_node object node\endlink and adds it to the node
 * at the top of the stack passed as an argument.
 * 
 * \param st A stack of DOM tree nodes containing the nodes on the path from the root of the tree
 * to the current node.
 */
extern void obj_start_cb(std::stack<json::internal_node *> *);
/**
 * \brief Pops the stack.
 * 
 * \param st A stack of DOM tree nodes containing the nodes on the path from the root of the tree
 * to the current node.
 */
extern void obj_end_cb(std::stack<json::internal_node *> *);
/**
 * \brief Creates a \link json::obj_node key:value pair node\endlink. If
 * the top of the stack contains another \link json::obj_node key:value pair node\endlink
 * then it pops the stack. The newly created node is added to the node on the top of the stack,
 * which is a \link json::obj_list_node object node\endlink.
 * 
 * \param st A stack of DOM tree nodes containing the nodes on the path from the root of the tree
 * to the current node.
 */
extern void key_cb(const std::string&, std::stack<json::internal_node *> *);
/**
 * \brief Creates a leaf node (\link json::string_node string node\endlink,
 * \link json::number_node number node\endlink, or \link json::bool_node string node\endlink)
 * and adds it to the node at the top of the stack.   
 *
 * \param token The token representing the string, number, or boolean 
 * \param st A stack of DOM tree nodes containing the nodes on the path from the root of the tree
 * to the current node.
 */
extern void obj_data_cb(const std::string&, int, std::stack<json::internal_node *> *);
/**
 * \brief Creates an \link json::array_node array node\endlink and adds it to the node
 * on the top of the stack.
 * 
 * \param st A stack of DOM tree nodes containing the nodes on the path from the root of the tree
 * to the current node.
 */
extern void array_start_cb(std::stack<json::internal_node *> *);
/**
 * \brief Pops the stack.
 * 
 * \param st A stack of DOM tree nodes containing the nodes on the path from the root of the tree
 * to the current node.
 */
extern void array_end_cb(std::stack<json::internal_node *> *);
/**
 * \brief Creates a leaf node (\link json::string_node string node\endlink,
 * \link json::number_node number node\endlink, or \link json::bool_node string node\endlink)
 * and adds it to the node at the top of the stack.   
 *
 * \param token The token representing the string, number, or boolean 
 * \param st A stack of DOM tree nodes containing the nodes on the path from the root of the tree
 * to the current node.
 */
extern void array_data_cb(const std::string&, int, std::stack<json::internal_node *> *);

#endif

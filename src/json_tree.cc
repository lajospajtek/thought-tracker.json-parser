#include <iostream>
#include <cassert>
#include <vector>
#include <stack>
#include <cstdlib>
#include <cstring>
#include "json_tree.hh"
#include "json_parser.hh"

namespace json {

array_node::~array_node() {
	for (std::vector<const json::node *>::iterator i = v.begin(); i != v.end(); ++i)
		if (0 != *i)
			delete *i;
}

root_node::~root_node() {
	if (0 != value_)
		delete value_;
}

obj_list_node::~obj_list_node() {
	for (std::vector<const json::obj_node *>::iterator i = v.begin(); i != v.end(); ++i)
		if (0 != *i)
			delete *i;
}

std::ostream&
string_node::print(std::ostream& os) const {
	return os << '"' << name_ << '"';
}

std::ostream&
number_node::print(std::ostream& os) const {
	return os << number_;
}

std::ostream&
bool_node::print(std::ostream& os) const {
	return os << (flag_ ? "true" : "false");
}

std::ostream&
root_node::print(std::ostream& os) const {
	if (0 == value_)
		return os << "null";
	else
		return value_->print(os);
}

std::ostream&
obj_node::print(std::ostream& os) const {
	os << '"' << name_ << "\" : ";
	return root_node::print(os);
}

std::ostream&
array_node::print(std::ostream& os) const {
	os << "[";
	std::vector<const node *>::const_iterator i = v.begin();
	if (i != v.end()) {
		if (0 == *i)
			os << "null";
		else
			(*i)->print(os);
		for (++i; i != v.end(); ++i) {
			os << ", ";
			if (0 == *i)
				os << "null";
			else
				(*i)->print(os);
		}
	}
	os << "]";
	return os;
}

std::ostream&
obj_list_node::print(std::ostream& os) const {
	os << "{";
	std::vector<const obj_node *>::const_iterator i = v.begin();
	if (i != v.end()) {
		(*i)->print(os);
		for (++i; i != v.end(); ++i) {
			os << ", ";
			(*i)->print(os);
		}
	}
	os << "}";
	return os;
}

}

std::ostream&
operator<<(std::ostream& os, const json::root_node& r) {
	return r.print(os);
}

void
obj_start_cb(std::stack<json::internal_node *> *st) {
	json::obj_list_node *ol = new json::obj_list_node();
	json::coll_node *n = dynamic_cast<json::coll_node *>(st->top());
	n->add(ol);
	if (0 != dynamic_cast<json::obj_node *>(n))
		st->pop();
	st->push(ol);
}

void
obj_end_cb(std::stack<json::internal_node *> *st) {
	st->pop();
}

void
key_cb(const std::string& name, std::stack<json::internal_node *> *st) {
	json::obj_node *obj = new json::obj_node(name);
	json::obj_list_node *r = dynamic_cast<json::obj_list_node *>(st->top());
	r->add(obj);
	st->push(obj);
}

void
obj_data_cb(const std::string& data, int term, std::stack<json::internal_node *> *st) {
	json::obj_node *obj = dynamic_cast<json::obj_node *>(st->top());
	if (json::scanner<char>::STRING == term)
		obj->add(data);
	else if (0 == strcasecmp("false", data.c_str()))
		obj->add(false);
	else if (0 == strcasecmp("true", data.c_str()))
		obj->add(true);
	else if (0 == strcasecmp("null", data.c_str()))
		obj->add(static_cast<const json::node *>(0));
	else
		obj->add(strtod(data.c_str(), 0));
	st->pop();
}

void
array_start_cb(std::stack<json::internal_node *> *st) {
	json::array_node *a = new json::array_node();
	json::coll_node *n = dynamic_cast<json::coll_node *>(st->top());
	n->add(a);
	if (0 != dynamic_cast<json::obj_node *>(n))
		st->pop();
	st->push(a);
}

void
array_end_cb(std::stack<json::internal_node *> *st) {
	st->pop();
}

void
array_data_cb(const std::string& data, int term, std::stack<json::internal_node *> *st) {
	json::array_node *a = dynamic_cast<json::array_node *>(st->top());
	if (json::scanner<char>::STRING == term)
		a->add(data);
	else if (0 == strcasecmp("false", data.c_str()))
		a->add(false);
	else if (0 == strcasecmp("true", data.c_str()))
		a->add(true);
	else if (0 == strcasecmp("null", data.c_str()))
		a->add(static_cast<const json::node *>(0));
	else
		a->add(strtod(data.c_str(), 0));
}

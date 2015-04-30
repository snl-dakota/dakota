/*  _________________________________________________________________________
 *
 *  UTILIB: A utility library for developing portable C++ codes.
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README file in the top UTILIB directory.
 *  _________________________________________________________________________
 */

/**
 * \file SimpleSplayTree.h
 *
 * Defines the utilib::SimpleSplayTreeItem and 
 * utilib::SimpleSplayTree classes
 */

#ifndef utilib_SimpleSplayTree_h
#define utilib_SimpleSplayTree_h

#include <utilib/_generic.h>
#include <utilib/AbstractSplayTree.h>
#include <utilib/compare.h>

namespace utilib {

template <class T>
class SimpleSplayTree;


/**
 * Defines the container class used for the SimpleSplayTree class.
 */
template <class T>
class SimpleSplayTreeItem 
{
  #if !defined(DOXYGEN)
  friend class AbstractSplayTree<SimpleSplayTreeItem<T>,T>;
  friend class SimpleSplayTree<T>;
  #endif

public:

  /// Return the size of the subtree.
  int Size() const {return size;}

  /// Write the key to an output stream.
  void write(std::ostream& os) const {os << Key;}

  /// Return the \c ctr counter.
  int counter() const {return ctr;}

  /// Return the key.
  T& key() {return Key;}

  /// Return the key.
  const T& key() const {return Key;}

  /// Compare the current key with \a key.
  int compare(const T& key) const
	{return utilib::compare(Key,key);}

  /// Clone the current object
  SimpleSplayTreeItem<T>* clone()
	{
	SimpleSplayTreeItem<T>* newtree = new SimpleSplayTreeItem<T>(&Key);
	newtree->Key = Key;
	newtree->ctr = ctr;
	newtree->size = size;
        if (left)
	   newtree->left = left->clone();
	if (right)
	   newtree->right = right->clone();
	return newtree;
	}

private:

  /// The key object.
  T Key;

  /// The number of instances of \c Key that have been inserted into this 
  /// splay tree.
  int ctr;
 
  /// Pointer to the left subtree.
  SimpleSplayTreeItem<T> *left;

  /// Pointer to the right subtree.
  SimpleSplayTreeItem<T> *right;

  /// The size of the subtree below this item and including this item.
  int size;

  /// Constructor, which requires a key.
  SimpleSplayTreeItem(const T* Key_) : Key(*Key_), ctr(1),left(0),right(0),
				  size(0) {}
};


/**
 * A splay tree class that uses standard C/C++ data types for keys.
 * The SimpleSplayTree class is derived from AbstractSplayTree, which
 * defines the basic operations of the splay tree.   This particular
 * instantiation of splay trees uses a simple data type for the key, and
 * no auxiliary data is associated with the key.  This greatly simplifies
 * the definition of the splay tree, but it assumes that copying the key
 * is relatively inexpensive (e.g. no memory allocation/deallocation is
 * required).
 *
 * \par
 * This template class can be instantiated with a data type for which the 
 * following methods are defined:
 *  - operator<
 *  - operator>
 *  - operator==
 *  - operator<<
 *  - operator<<(ostream& os)
 * 
 * \sa GenericSplayTree
 */
template <class T>
class SimpleSplayTree : public AbstractSplayTree<SimpleSplayTreeItem<T>,T>
{
public:

  #if !defined(DOXYGEN)
  typedef AbstractSplayTree<SimpleSplayTreeItem<T>,T> base_t;
  #endif

  /// Constructor, which specifies a name for the tree.
  SimpleSplayTree(const char* nameBuff = "Unnamed")
	: AbstractSplayTree<SimpleSplayTreeItem<T>,T>(nameBuff), 
		duplicate_flag(true), check_duplicates_flag(true) {}

  /// Copy constructor
  SimpleSplayTree(const SimpleSplayTree<T>& tree)
	{ *this = tree; }

  /// Copy equal operator
  SimpleSplayTree<T>& operator=(const SimpleSplayTree<T>& t)
	{
	base_t::operator=(t);
	duplicate_flag = t.duplicate_flag;
	check_duplicates_flag = t.check_duplicates_flag;
	return *this;
	}

  /// Returns the value of \c duplicate_flag.
  bool& duplicate() {return duplicate_flag;}

  /// Returns the value of \c check_duplicates_flag.
  bool& check_duplicates() {return check_duplicates_flag;}

protected:
 
  /// Records whether the last item inserted was a duplicate
  bool duplicate_flag;

  /// If \c true then track items in the tree that are duplicates
  bool check_duplicates_flag;

  ///
  SimpleSplayTreeItem<T>* insert(T* key);

  ///
  void extract(SimpleSplayTreeItem<T>* item, bool& status);

};


template <class T>
SimpleSplayTreeItem<T>* SimpleSplayTree<T>::insert(T* key)
{
this->tree = exec_splay(key,this->tree);
SimpleSplayTreeItem<T>* treeitem = this->top();

duplicate_flag = false;
if (check_duplicates_flag && (treeitem != NULL)) {
   if (treeitem->compare(*key)==0) {	// it's already there
      treeitem->ctr++;
      duplicate_flag = true;
      return treeitem;
      }
   }
return AbstractSplayTree<SimpleSplayTreeItem<T>,T>::insert(key);
}


template <class T>
void SimpleSplayTree<T>::extract(SimpleSplayTreeItem<T>* item, bool& status)
{
if (check_duplicates_flag) {
   this->tree = exec_splay(&(item->key()),this->tree);
   SimpleSplayTreeItem<T>* treeitem = this->top();
   if (treeitem && (treeitem->compare(item->key()) == 0)) {
      treeitem->ctr--;
      if (treeitem->ctr > 0) {
         status=true;
         return;
         }
      }
   }
AbstractSplayTree<SimpleSplayTreeItem<T>,T>::extract(item,status);
}

} // namespace utilib

#endif

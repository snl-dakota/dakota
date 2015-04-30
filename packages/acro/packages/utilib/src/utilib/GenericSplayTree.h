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
 * \file GenericSplayTree.h
 *
 * Defines the utilib::GenericSplayTreeItem and utilib::GenericSplayTree
 * classes
 */

#ifndef utilib_GenericSplayTree_h
#define utilib_GenericSplayTree_h

#include <utilib_config.h>
#include <utilib/_generic.h>
#include <utilib/AbstractSplayTree.h>
 
namespace utilib {


///
template <class T>
class GenericSplayTree;


/**
 * An item in a utilib::GenericSplayTree
 */
template <class T>
class GenericSplayTreeItem
{
  #if !defined(DOXYGEN)
  friend class AbstractSplayTree<GenericSplayTreeItem<T>,T>;
  friend class GenericSplayTree<T>;
  #endif

public:

  /// Return the size of the subtree
  int Size() {return size;}

  /// Write the key to an output stream.
  void write(std::ostream& os) const {Keyptr->write(os);}

  /// Return the key.
  T& key() {return *Keyptr;}

  ///
  /// Return the key.
  const T& key() const {return *Keyptr;}

  /// Compare the current item with the value of the given key
  int compare(const T& key) const
		{return Keyptr->compare(key);}

  /// Create a copy of this object
  GenericSplayTreeItem<T>* clone()
        {
        GenericSplayTreeItem<T>* newtree = new GenericSplayTreeItem<T>(Keyptr);
        newtree->Keyptr = Keyptr;
        newtree->ctr = ctr;
        newtree->size = size;
	if (left)
           newtree->left = left->clone();
	if (right)
           newtree->right = right->clone();
        return newtree;
        }

private:

  /// A pointer to a key object.
  T* Keyptr;

  /// The number of instances of \c Key that have been inserted into this splay tree.
  int ctr;

  /// Pointer to the left subtree.
  GenericSplayTreeItem<T> *left;

  /// Pointer to the right subtree.
  GenericSplayTreeItem<T> *right;

  /// The size of the subtree below this item and including this item.
  int size;

  /// Constructor, which requires a key.
  GenericSplayTreeItem(T* key_ptr_) : Keyptr(key_ptr_), ctr(1), left(0),
				right(0), size(0) {}
};



/**
 * A splay tree class that uses a general set of classes for keys.
 * The GenericSplayTree class is derived from AbstractSplayTree,
 * which defines the basic operations of the splay tree. The keys
 * are assumed to be classes for which the following operations are
 * defined:
 *	int compare(const KEY* key) const
 *	int write(ostream& os) const
 *	int read(istream& is)
 *
 * \sa SimpleSplayTree
 */
template <class T>
class GenericSplayTree : public AbstractSplayTree<GenericSplayTreeItem<T>,T>
{
public:

  #if !defined(DOXYGEN)
  ///
  typedef AbstractSplayTree<GenericSplayTreeItem<T>,T> base_t;
  #endif

  /// Constructor, which specifies a name for the tree.
  explicit GenericSplayTree(const char* nameBuff = "Unnamed")
	: AbstractSplayTree<GenericSplayTreeItem<T>,T>(nameBuff), 
		duplicate_flag(true), check_duplicates_flag(true) {}

  /// Copy constructor
  GenericSplayTree(const GenericSplayTree<T>& tree)
	{ *this = tree; }

  /// Copy operator
  GenericSplayTree<T>& operator=(const GenericSplayTree<T>& t)
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
 
  /// Flag that is true if duplicates are allowed
  bool duplicate_flag;

  /// Flag that is true if the splay tree looks for duplicates
  bool check_duplicates_flag;

  /**
   * Insert \a key into the splay tree and return the item used for 
   * it in the tree.
   */
  GenericSplayTreeItem<T>* insert(T* key);

  /**
   * Remove \a item from the tree.  Set the value of \a status to true
   * if the item existed in the tree.
   */
  void extract(GenericSplayTreeItem<T>* item, bool& status);

};


template <class T>
GenericSplayTreeItem<T>* GenericSplayTree<T>::insert(T* key)
{
this->tree = exec_splay(key,this->tree);
GenericSplayTreeItem<T>* treeitem = this->top();

duplicate_flag = false;
if (check_duplicates_flag && (treeitem != NULL)) {
   if (treeitem->compare(*key)==0) {     // it's already there
      treeitem->ctr++;
      duplicate_flag = true;
      return treeitem;
      }
   }
return AbstractSplayTree<GenericSplayTreeItem<T>,T>::insert(key);
}


template <class T>
void GenericSplayTree<T>::extract(GenericSplayTreeItem<T>* item, bool& status)
{
if (check_duplicates_flag) {
   this->tree = exec_splay(&(item->key()),this->tree);
   GenericSplayTreeItem<T>* treeitem = this->top();
   if (treeitem && (treeitem->compare(item->key()) == 0)) {
      treeitem->ctr--;
      if (treeitem->ctr > 0) {
         status=true;
         return;
         }
      }
   }
AbstractSplayTree<GenericSplayTreeItem<T>,T>::extract(item,status);
}

} // namespace utilib

#endif

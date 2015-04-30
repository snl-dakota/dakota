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
 * \file AbstractSplayTree.h
 *
 * Defines the utilib::AbstractSplayTree class
 */

#ifndef utilib_AbstractSplayTree_h
#define utilib_AbstractSplayTree_h

#include <utilib_config.h>
#include <utilib/std_headers.h>

#ifdef _MSC_VER
/* TODO - Try compiling with msvc and find out if this is needed.*/
#include "crtdbg.h"
#endif

#include <utilib/_generic.h>
#include <utilib/AbstractSplayTree.h>
#include <utilib/exception_mngr.h>
#include <utilib/compare.h>
#include <utilib/PackBuf.h>

namespace utilib {

/// A simple macro used internally for the splay tree code.
#define node_size(x)   (((x)==NULL) ? 0 : ((x)->size))


/**
 * Implements an abstract class for defining the core operations of a
 * top-down splay tree.
 *
 * This is adapted from code developed by D. Sleator, which itself is 
 * adapted from simple top-down splay, at the bottom of 669 of 
 * \if GeneratingLaTeX Sleator and Tarjan~\cite{SleTar85}. \endif
 * \if GeneratingHTML SleTar85. \endif
 * Sleator's code can be aquired from
 *
 *   http://www-cgi.cs.cmu.edu/afs/cs/user/sleator/www/home.html
 */
template <class T, class KEY>
class AbstractSplayTree
{
public:

  /// Constructor, which specifies a name for the tree.
  explicit AbstractSplayTree(const char* nameBuff = "Unnamed");

  /// Destructor.
  virtual ~AbstractSplayTree() {clear();}
  
  /// Copy operator
  AbstractSplayTree<T,KEY>& operator=(const AbstractSplayTree<T,KEY>& tree);

  /// Returns \c true if the tree is empty and \c false otherwise.
  bool empty() const {return (tree == 0);}

  /// Returns \c true if the tree is not empty and \c false otherwise.
  operator bool() const {return (tree != 0);}

  /// Returns the size of the splay tree.
  int size() const { return (tree ? tree->size : 0); }

  /// Return the value of \c Sense.
  OrderSense sense() {return Sense;}

  /// Set the value of \c Sense.
  void setSense(OrderSense sense_) {Sense=sense_;}

  /// Perform a splay operation starting from the root of the tree.
  void splay(const KEY& key)
		{tree = exec_splay(&key,tree);}

  /**
   * Return the item in the splay tree with rank \a r.
   * Returns a null item if a bad rank value is passed in.
   */
  virtual T* find_rank(int r);

  /**
   * Return the item in the splay tree with rank \a r.
   * Returns a null item if a bad rank value is passed in.
   */
  virtual const T* find_rank(int r) const
	{
	AbstractSplayTree<T,KEY>* Tthis = const_cast< AbstractSplayTree<T,KEY>* >(this);
	T* tmp = Tthis->find_rank(r);
	return tmp;
	}

  /**
   * Return the item in the splay tree with the given key.
   * Returns null if the tree is empty or if the item is not
   * in the tree.
   */
  virtual T* find(const KEY& key)
		{
		splay(key);
		if (!tree) return tree;
		if (tree->compare(key) == 0) return tree;
		else                          return (T*)0;
		}

  /// Return the top of the splay tree.
  T* top() 	{return tree;}

  /// Return the top of the splay tree.
  const T* top() const 	{return tree;}

  /**
   * Add a key to the splay tree and return the splay tree item
   * that contains it.
   */
  virtual T* add(KEY& key)
		{return insert(&key);}

  /**
   * Remove a splay tree item with the given key.
   * The status flag is \c true if the key was found and \c false
   * otherwise.
   */
  virtual void remove(KEY& key, bool& status)
		{
		T* tmp = find(key);
		if (tmp) extract(tmp,status);
		else	 status = false;
		}

  /**
   * Remove a splay tree item.
   * The status flag is \c true if the item was found and \c false
   * otherwise.
   */
  virtual void remove(T*& item, bool& status)
		{
		if (!item)
		   status = false;
		else {
		   extract(item,status);
		   item = 0;
		   }
		}

  /**
   * Remove a splay tree item and return the item's key.
   * The status flag is \c true if the item was found and \c false
   * otherwise.
   */
  virtual void remove(T*& item, KEY& key, bool& status)
		{
		if (!item)
	   	   status = false;
		else {
		   key = item->key();
		   extract(item,status);
		   item = 0;
		   }
		}

  /// Returns the rank of the top (0...size-1)
  int rank(const KEY& key)
		{
		splay(key);
		if (!tree) return 0;
		return (tree->left ? tree->left->size : 0);
		}

  ///
  void balance() const
		{
		int n = size()/2;
		find_rank(n);
		}

  /// Empty out a splay tree.
  void clear(T* ptr=0);

protected:

//  void print_tree(ostream& os, int d, T* item);


  /**
   * Search for an item with a given key on a tree rooted at \a item.  
   * If an item is found in the tree, it is splayed to the root.
   * Otherwise, the item put at the root is the last one before \c NULL 
   * that would have been reached in a normal binary search.  
   * (It is a neighbor of i in the tree.)  This method of splaying 
   * is very convenient for facilitating a variety of other operations.
   */
  T* exec_splay(const KEY* key, T* item);

  /// Add a key to the tree.
  virtual T* insert(KEY* key);

  /**
   * Remove a splay tree item.
   * The status flag is \c true if the item was found and \c false
   * otherwise.
   */
  virtual void extract(T* item, bool& status);

  /// The current root of the splay tree.
  T* tree;

  /// Specifies the ordering within the tree: increasing or decreasing.
  OrderSense Sense;

  /// The name of the class instance.
  const char* name;

};


 


template <class T, class KEY>
AbstractSplayTree<T,KEY>::AbstractSplayTree(const char* nameBuff)
	: tree(0),
	  Sense(increasing),
	  name(nameBuff)
{
if (!name)
   name = "";
}



template <class T, class KEY>
AbstractSplayTree<T,KEY>& 
	AbstractSplayTree<T,KEY>::operator=(
					const AbstractSplayTree<T,KEY>& t)
{
if (t.tree) {
   // Balance the tree first to avoid pathological behavior
   t.balance();
   tree = t.tree->clone();
   }
else
   tree = 0;
Sense = t.Sense;
name = "";
return *this;
}


template <class T, class KEY>
T* AbstractSplayTree<T,KEY>::exec_splay(const KEY* key, T* t)
{
    T N((KEY*)key), *l, *r, *y;
    int comp, l_size, r_size;
    
    if (t == 0) return t;
    comp = Sense*t->compare(*key);
    if (comp == 0) return t;

    N.left = N.right = NULL;
    l = r = &N;
    l_size = r_size = 0;
 
    while (1) {
        if (comp > 0) {
            if (t->left == NULL) break;
            if ((Sense*t->left->compare(*key)) > 0) {
                y = t->left;                           /* rotate right */
                t->left = y->right;
                y->right = t;
                t->size = node_size(t->left) + node_size(t->right) + 1;
                t = y;
                if (t->left == NULL) break;
            }
            r->left = t;                               /* link right */
            r = t;
            t = t->left;
            r_size += 1+ node_size(r->right);
        } else if (comp < 0) {
            if (t->right == NULL) break;
            if ((Sense*t->right->compare(*key)) < 0) {
                y = t->right;                          /* rotate left */
                t->right = y->left;
                y->left = t;
		t->size = node_size(t->left) + node_size(t->right) + 1;
                t = y;
                if (t->right == NULL) break;
            }
            l->right = t;                              /* link left */
            l = t;
            t = t->right;
            l_size += 1+ node_size(l->left);
        } else {
            break;
        }
        comp = Sense*t->compare(*key);
    }
    l_size += node_size(t->left);  /* Now l_size and r_size are the sizes of */
    r_size += node_size(t->right); /* the left and right trees we just built.*/
    t->size = l_size + r_size + 1;

    l->right = r->left = NULL;

    /* The following two loops correct the size fields of the right path  */
    /* from the left child of the root and the right path from the left   */
    /* child of the root.                                                 */
    for (y = N.right; y != NULL; y = y->right) {
        y->size = l_size;
        l_size -= 1+ node_size(y->left);
    }
    for (y = N.left; y != NULL; y = y->left) {
        y->size = r_size;
        r_size -= 1+ node_size(y->right);
    }
 
    l->right = t->left;                                /* assemble */
    r->left = t->right;
    t->left = N.right;
    t->right = N.left;

    return t;
}



template <class T, class KEY>
T* AbstractSplayTree<T,KEY>::insert(KEY* newkey)
{
    T* newroot = new T(newkey);
    if (tree == NULL) {
	newroot->left = newroot->right = NULL;
        }
    else {
        if ((Sense*newroot->compare(tree->key())) < 0) {
	   newroot->left = tree->left;
	   newroot->right = tree;
	   tree->left = NULL;
	   tree->size = 1+ node_size(tree->right);
        } else {
	   newroot->right = tree->right;
	   newroot->left = tree;
	   tree->right = NULL;
	   tree->size = 1+ node_size(tree->left);
        }
    }
    
    newroot->size = 1 + node_size(newroot->left) + node_size(newroot->right);
    tree = newroot;
    return tree;
}



template <class T, class KEY>
void AbstractSplayTree<T,KEY>::extract(T* node, bool& status) {
    T* x;
    int tsize;

    if (tree==NULL) return;
    tsize = tree->size;
    tree = exec_splay(&(node->key()),tree);
    if ((Sense*node->compare(tree->key())) == 0) {               // found it
	status = true;
	if (tree->left == NULL) {
	    x = tree->right;
	} else {
	    x = exec_splay(&(node->key()), tree->left);
	    x->right = tree->right;
	}
	delete tree;
	if (x != NULL) {
	    x->size = tsize-1;
	}
	tree = x;
    }
    else {
        status = false;
        node = 0;
    }
}



template <class T, class KEY>
T* AbstractSplayTree<T,KEY>::find_rank(int r)
{
    T* tmp=tree;
    int lsize;
    if ((r < 0) || (r >= node_size(tmp))) return NULL;
    for (;;) {
	lsize = node_size(tmp->left);
	if (r < lsize) {
	    tmp = tmp->left;
	} else if (r > lsize) {
	    r = r - lsize - 1;
	    tmp = tmp->right;
	} else {
	    break;
	}
    }
    tree = exec_splay(&(tmp->key()),tree);
    return tmp;
}

#if 0
void AbstractSplayTree<T,KEY>::print_tree(ostream& os, int d, 
					T* t) {
    int i;
    if (tree == NULL) return;
    print_tree(os,d+1,tree->right);
    for (i=0; i<d; i++) os << "  ";
    dowrite(tree,os);
    //os << "(" << node_size(tree) << "," << tree->ctr << ")" << endl;
    print_tree(os,d+1,tree->left);
}
#endif


template <class T, class KEY>
void AbstractSplayTree<T,KEY>::clear(T* ptr)
{
if (!ptr) {
   if (tree) {
      // Before we clear the tree, we rebalance it
      // This prevents pathological cases where clear recurses so 
      // long that it eats up the stack!
      balance();
      // Clear the tree
      clear(tree);
      tree=0;
      }
   return;
   }
if (ptr->left)
   clear(ptr->left);
if (ptr->right)
   clear(ptr->right);
delete ptr;
}

#undef node_size

} // namespace utilib

/// Out-stream operator for writing the contents of a AbstractSplayTree
template <class T, class KEY>
inline std::ostream& operator<<(std::ostream& os, const utilib::AbstractSplayTree<T,KEY>& obj)
{
os << obj.size() << std::endl;

const T* node = obj.find_rank(0);
int i=1;
while (node) {
  //os << node->ctr << " ";
  node->write(os);
  os << std::endl;
  node = obj.find_rank(i++);
  }
return os;
}

/// Out-stream operator for writing the contents of a AbstractSplayTree
template <class T, class KEY>
inline utilib::PackBuffer& operator<<(utilib::PackBuffer& os,
                                        const utilib::AbstractSplayTree<T,KEY>& obj)
{
os << obj.size();

const T* node = obj.find_rank(0);
int i=1;
while (node) {
  node->write(os);
  node = obj.find_rank(i++);
  }
return os;
}


/// In-stream operator for reading the contents of a AbstractSplayTree
template <class T, class KEY>
inline std::istream& operator>>(std::istream& is, utilib::AbstractSplayTree<T,KEY>& obj)
{
int Size;
is >> Size;
for (int i=0; i<Size; i++) {
  T* item = 0;
  item->read(is);
  obj.add(item);
  }
return is;
}

/// In-stream operator for reading the contents of a AbstractSplayTree
template <class T, class KEY>
inline utilib::UnPackBuffer& operator>>(utilib::UnPackBuffer& is,
                                        utilib::AbstractSplayTree<T,KEY>& obj)
{
int Size;
is >> Size;
for (int i=0; i<Size; i++) {
  T* item = 0;
  item->read(is);
  obj.add(item);
  }
return is;
}



#endif

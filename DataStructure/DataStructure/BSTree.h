#ifndef __BSTREE_HH__
#define __BSTREE_HH__
#endif // !__BSTREE_HH__

#include <iostream>
//BSTree类模板声明 容器类 封装主要的API
template<class T>
class  BSTree
{
private:
	BNode<T>* d_root; //整个树的根
	int d_height; //当前树的高度
	int d_size; //当前元素的高度
	/// <summary>
	/// 内部递归插入算法
	/// </summary>
	/// <param name=""></param>
	/// <param name=""></param>
	/// <returns></returns>
	BNode<T>* insertRcu(BNode<T>*,T);
	void clear(BNode<T>*);
	bool search(BNode<T>*,const T&);
	//最大高度
	int maxHeight(BNode<T>*);

	//级别遍历所有节点
	void level_visit(BNode<T>*);

	//void level_visit(BNode<T>*);
};

//BSTree迭代器类模板声明
template <class T>
class BSTreeIterator;

template <class T>
class BNode
{
	/// <summary>
	/// 目的访问到 BSTree 的私有成员和protected成员
	/// </summary>
	friend class BSTree<T>;
private:
	BNode<T>* d_parent; //父节点
	BNode<T>* d_left; //左子节点
	BNode<T>* d_right; //右子节点
	T d_data; //数据域
public:
	BNode(T val)
	{
		d_data = val;
		d_parent = nullptr;
		d_left = nullptr;
		d_right = nullptr;
	}
	~BNode()
	{
		d_parent = nullptr;
		d_left = nullptr;
		d_right = nullptr;
	}
};




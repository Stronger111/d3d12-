#ifndef __BSTREE_HH__
#define __BSTREE_HH__
#endif // !__BSTREE_HH__

#include <iostream>
//BSTree��ģ������ ������ ��װ��Ҫ��API
template<class T>
class  BSTree
{
private:
	BNode<T>* d_root; //�������ĸ�
	int d_height; //��ǰ���ĸ߶�
	int d_size; //��ǰԪ�صĸ߶�
	/// <summary>
	/// �ڲ��ݹ�����㷨
	/// </summary>
	/// <param name=""></param>
	/// <param name=""></param>
	/// <returns></returns>
	BNode<T>* insertRcu(BNode<T>*,T);
	void clear(BNode<T>*);
	bool search(BNode<T>*,const T&);
	//���߶�
	int maxHeight(BNode<T>*);

	//����������нڵ�
	void level_visit(BNode<T>*);

	//void level_visit(BNode<T>*);
};

//BSTree��������ģ������
template <class T>
class BSTreeIterator;

template <class T>
class BNode
{
	/// <summary>
	/// Ŀ�ķ��ʵ� BSTree ��˽�г�Ա��protected��Ա
	/// </summary>
	friend class BSTree<T>;
private:
	BNode<T>* d_parent; //���ڵ�
	BNode<T>* d_left; //���ӽڵ�
	BNode<T>* d_right; //���ӽڵ�
	T d_data; //������
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




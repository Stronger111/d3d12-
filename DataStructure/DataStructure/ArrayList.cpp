#include "ArrayList.h"
/*
*��̬���� ��ֻ�����ڴ����ݵĹ��̻����ڴ����ݵĹ��� 
* d_capacity/d_size�ı�ֵ������ñ�ֵ�ﵽ3 ����װ��Ԫ�رȽ��ٵ�ʱ�����Ҫ����
* 
*/
/// <summary>
/// Ĭ�Ϲ��캯��
/// </summary>
/// <typeparam name="T"></typeparam>
template <typename T>
ArrayList<T>::ArrayList()
{
	d_size = 0;
	//15��Ԫ�س��ȴ�С
	d_capacity = CAPACITY;
	d_arr = new T[d_capacity];
}

//�Զ���Ĺ��캯����������
template <typename T>
ArrayList<T>::ArrayList(const size_t &&n):d_size(n)
{
	set_capacity(n);
	d_arr = new T[d_capacity];
}

//�Զ���Ĺ��캯��
template <class T>
ArrayList<T>::ArrayList(const T *data,size_t n):d_capacity(CAPACITY)
{
	if (n>0)
	{
		std::cout << "Array List ��ͨ��������" << std::endl;
		d_arr = new T[n];
		for (size_t i = 0; i < n; i++)
		{
			*(d_arr + i) = *(data+i);
		}
		d_size = n;
	}
}

//copy���캯��
template <class T>
ArrayList<T>::ArrayList(const ArrayList &oth):d_size(oth.d_size)
{
	std::cout << "ArrayList copy ��������" << std::endl;
	set_capacity(oth.d_size);
	d_arr = new T[d_capacity];

	for (size_t i = 0; i < oth.d_size; i++)
	{
		*(d_arr + i) = *(oth.d_arr+i);
	}
}
//move���캯��
template <class T>
ArrayList<T>::ArrayList(ArrayList&& oth) : d_size(oth.d_size)
{
	std::cout << "ArrayList move��������" << std::endl;
	d_arr = oth.d_arr;
	oth.d_size = 0;
	oth.d_arr = nullptr;
}

//�ڴ����
template <class T>
ArrayList<T>::~ArrayList()
{
	std::cout << "ArrayList��������" << std::endl;
	if (d_arr!=nullptr)
	{
		delete[] d_arr;
	}
}

//�ڲ����ݲ���
template <class T>
void ArrayList<T>::expand()
{
	d_capacity = d_size * EXP_FACTOR;
	T* tmp = new T[d_capacity];
	for (size_t i = 0; i < d_size; i++)
	{
		*(tmp + i) = *(d_arr + i);
	}
	//�Ӿɵ������ڴ濽�����µ������ڴ� ��ԭ���ɵ��ڴ�ռ��ͷŷ�����Ŷ�ڴ��
	delete[] d_arr;
	d_arr = temp;
}

//�ڴ���������
template <class T>
void ArrayList<T>::shrink()
{
	size_t k = d_capacity / d_size;
	//����3
	if (k>=SHK_FACTOR)
	{
		d_capacity = d_size * EXP_FACTOR;
		T* tmp = new T[d_capacity];
		for (size_t i = 0; i < size_t; i++)
		{
			*(tmp + i) = *(d_arr+i);
		}

		delete[] d_arr;
		d_arr = tmp;
	}
}

template <class T>
inline long ArrayList<T>::cvt_signed_number(size_t n)
{
	size_t x = static_cast<size_t>(std::numeric_limits<long>::max());

	if (n > x)
	{
		clean();
		throw std::overflow_error("����nת������");
	}
	return static_cast<long>(n);
}


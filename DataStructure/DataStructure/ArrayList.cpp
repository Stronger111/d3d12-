#include "ArrayList.h"
/*
*动态数组 不只是有内存扩容的过程还有内存缩容的过程 
* d_capacity/d_size的比值，如果该比值达到3 里面装的元素比较少的时候就需要缩容
* 
*/
/// <summary>
/// 默认构造函数
/// </summary>
/// <typeparam name="T"></typeparam>
template <typename T>
ArrayList<T>::ArrayList()
{
	d_size = 0;
	//15个元素长度大小
	d_capacity = CAPACITY;
	d_arr = new T[d_capacity];
}

//自定义的构造函数设置容量
template <typename T>
ArrayList<T>::ArrayList(const size_t &&n):d_size(n)
{
	set_capacity(n);
	d_arr = new T[d_capacity];
}

//自定义的构造函数
template <class T>
ArrayList<T>::ArrayList(const T *data,size_t n):d_capacity(CAPACITY)
{
	if (n>0)
	{
		std::cout << "Array List 普通函数调用" << std::endl;
		d_arr = new T[n];
		for (size_t i = 0; i < n; i++)
		{
			*(d_arr + i) = *(data+i);
		}
		d_size = n;
	}
}

//copy构造函数
template <class T>
ArrayList<T>::ArrayList(const ArrayList &oth):d_size(oth.d_size)
{
	std::cout << "ArrayList copy 函数调用" << std::endl;
	set_capacity(oth.d_size);
	d_arr = new T[d_capacity];

	for (size_t i = 0; i < oth.d_size; i++)
	{
		*(d_arr + i) = *(oth.d_arr+i);
	}
}
//move构造函数
template <class T>
ArrayList<T>::ArrayList(ArrayList&& oth) : d_size(oth.d_size)
{
	std::cout << "ArrayList move函数调用" << std::endl;
	d_arr = oth.d_arr;
	oth.d_size = 0;
	oth.d_arr = nullptr;
}

//内存回收
template <class T>
ArrayList<T>::~ArrayList()
{
	std::cout << "ArrayList析构函数" << std::endl;
	if (d_arr!=nullptr)
	{
		delete[] d_arr;
	}
}

//内部扩容操作
template <class T>
void ArrayList<T>::expand()
{
	d_capacity = d_size * EXP_FACTOR;
	T* tmp = new T[d_capacity];
	for (size_t i = 0; i < d_size; i++)
	{
		*(tmp + i) = *(d_arr + i);
	}
	//从旧的数组内存拷贝到新的数组内存 把原来旧的内存空间释放返还给哦内存池
	delete[] d_arr;
	d_arr = temp;
}

//内存收缩过程
template <class T>
void ArrayList<T>::shrink()
{
	size_t k = d_capacity / d_size;
	//大于3
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
		throw std::overflow_error("参数n转换错误");
	}
	return static_cast<long>(n);
}


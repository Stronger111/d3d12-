#define CAPACITY 15
#define SHK_FACTOR 3
#define EXP_FACTOR 2
/// <summary>
/// 动态数组
/// </summary>
template <typename T>
class ArrayList
{
private:
	T* d_arr;
	size_t d_capacity;
	size_t d_size;

	void expand(void);
	void shrink(void);
	long cvt_signed_number(size_t n);

	inline void set_capacity(size_t n);

public:
	/// <summary>
	/// 默认的构造函数
	/// </summary>
	ArrayList();
	/// <summary>
	/// 自定义构造函数 右值引用 解析
	/// http://c.biancheng.net/view/1510.html
	/// </summary>
	/// <param name="n"></param>
	ArrayList(const size_t &&n);
	/// <summary>
	/// 自定义构造函数
	/// </summary>
	/// <param name="n"></param>
	ArrayList(const T *data,size_t n);

	//copy构造函数
	ArrayList(const ArrayList& oth);
	/// <summary>
	/// move构造函数
	/// </summary>
	/// <param name="oth"></param>
	ArrayList(ArrayList &&oth):d_size(oth.d_size);
	~ArrayList();
	/// <summary>
	/// 下标访问
	/// </summary>
	/// <param name="n"></param>
	/// <returns></returns>
	T& operator[](long n);
	/// <summary>
	/// ArrayList尺寸
	/// </summary>
	/// <returns></returns>
	size_t size()const;
	/// <summary>
	/// ArrayList当前分配内存书
	/// </summary>
	/// <returns></returns>
	size_t capacity() const;
	/// <summary>
	/// 交换两个ArrayList 对象
	/// </summary>
	/// <param name="oth"></param>
	void swap(ArrayList &oth);
	/// <summary>
	/// 尾部插入
	/// </summary>
	/// <param name="v"></param>
	void push_back(const T &v);
	void show_array();
	/// <summary>
	/// 查找特定元素值的索引,约定返回size_t 的上限值表示找不到元素
	/// </summary>
	/// <param name="v"></param>
	/// <returns></returns>
	inline size_t find(const T &v);
	/// <summary>
	/// 按值删除
	/// </summary>
	/// <param name="v"></param>
	/// <returns></returns>
	bool remove(const T &v);
	/// <summary>
	/// 删除指定索引的值
	/// </summary>
	/// <param name="n"></param>
	void removeAt(long n);
	/// <summary>
	/// 按索引位置插入值
	/// </summary>
	/// <param name="ix"></param>
	/// <param name="v"></param>
	void insert(size_t ix,T v);
	/// <summary>
	/// 尾部弹出数据
	/// </summary>
	/// <returns></returns>
	T pop();
	/// <summary>
	/// 清空整个容器
	/// </summary>
	void clean();
};
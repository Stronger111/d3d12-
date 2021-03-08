#define CAPACITY 15
#define SHK_FACTOR 3
#define EXP_FACTOR 2
/// <summary>
/// ��̬����
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
	/// Ĭ�ϵĹ��캯��
	/// </summary>
	ArrayList();
	/// <summary>
	/// �Զ��幹�캯�� ��ֵ���� ����
	/// http://c.biancheng.net/view/1510.html
	/// </summary>
	/// <param name="n"></param>
	ArrayList(const size_t &&n);
	/// <summary>
	/// �Զ��幹�캯��
	/// </summary>
	/// <param name="n"></param>
	ArrayList(const T *data,size_t n);

	//copy���캯��
	ArrayList(const ArrayList& oth);
	/// <summary>
	/// move���캯��
	/// </summary>
	/// <param name="oth"></param>
	ArrayList(ArrayList &&oth):d_size(oth.d_size);
	~ArrayList();
	/// <summary>
	/// �±����
	/// </summary>
	/// <param name="n"></param>
	/// <returns></returns>
	T& operator[](long n);
	/// <summary>
	/// ArrayList�ߴ�
	/// </summary>
	/// <returns></returns>
	size_t size()const;
	/// <summary>
	/// ArrayList��ǰ�����ڴ���
	/// </summary>
	/// <returns></returns>
	size_t capacity() const;
	/// <summary>
	/// ��������ArrayList ����
	/// </summary>
	/// <param name="oth"></param>
	void swap(ArrayList &oth);
	/// <summary>
	/// β������
	/// </summary>
	/// <param name="v"></param>
	void push_back(const T &v);
	void show_array();
	/// <summary>
	/// �����ض�Ԫ��ֵ������,Լ������size_t ������ֵ��ʾ�Ҳ���Ԫ��
	/// </summary>
	/// <param name="v"></param>
	/// <returns></returns>
	inline size_t find(const T &v);
	/// <summary>
	/// ��ֵɾ��
	/// </summary>
	/// <param name="v"></param>
	/// <returns></returns>
	bool remove(const T &v);
	/// <summary>
	/// ɾ��ָ��������ֵ
	/// </summary>
	/// <param name="n"></param>
	void removeAt(long n);
	/// <summary>
	/// ������λ�ò���ֵ
	/// </summary>
	/// <param name="ix"></param>
	/// <param name="v"></param>
	void insert(size_t ix,T v);
	/// <summary>
	/// β����������
	/// </summary>
	/// <returns></returns>
	T pop();
	/// <summary>
	/// �����������
	/// </summary>
	void clean();
};
#ifndef SUMTREE_H
#define SUMTREE_H
namespace agent
{
	class Sumtree
	{
	private:
		double *_data; // binary tree. Size = 2 * _capacity -1
		int _capacity;
		int _dataptr; // from 0 to _capacity
	public:
		Sumtree()
		{
			_capacity = 0;
			_data = nullptr;
			_dataptr = 0;
		}
		Sumtree(int capacity)
		{
			_capacity = capacity;
			_data = new double[2 * capacity - 1];
			for (int i = 0; i < 2 * _capacity - 1; i++)
			{
				_data[i] = 0.0;
			}
			_dataptr = 0;
		};
		Sumtree(const Sumtree &other)
		{
			_capacity = other._capacity;

			if (_data != nullptr)
			{
				delete[] _data;
			}
			_data = new double[2 * _capacity - 1];
			for (int i = 0; i < 2 * _capacity - 1; i++)
			{
				_data[i] = other._data[i];
			}
			_dataptr = other._dataptr;
		}
		~Sumtree()
		{
			if (_data != nullptr)
				delete[] _data;
		};
		void add(double p);
		void update(int tree_idx, double p);
		void getLeaf(double value, int &targetidx, double &leafValue) const;

		double getTotalP() const { return _data[0]; }
		double getMinP() const;
		int capacity() const { return _capacity; }
	};
}
#endif

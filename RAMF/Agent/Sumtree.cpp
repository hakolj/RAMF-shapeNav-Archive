#include "Sumtree.h"
namespace agent
{
	void Sumtree::add(double p)
	{
		int tree_idx = _dataptr + _capacity - 1;
		update(tree_idx, p);
		_dataptr += 1;
		if (_dataptr >= _capacity)
			_dataptr = 0;
	}

	void Sumtree::update(int tree_idx, double p)
	{
		double change = p - _data[tree_idx];
		_data[tree_idx] = p;
		// ���ϱ������½ڵ��ֵ
		while (tree_idx != 0)
		{
			tree_idx = (tree_idx - 1) / 2;
			_data[tree_idx] += change;
		}
	}

	void Sumtree::getLeaf(double value, int &targetidx, double &leafValue) const
	{
		int parentidx = 0;
		int clidx, cridx;
		int datalen = 2 * _capacity - 1;
		int leafidx = -1;
		while (true)
		{
			clidx = 2 * parentidx + 1;
			cridx = clidx + 1;

			if (clidx >= datalen)
			{
				// reach the bottom, end search
				leafidx = parentidx;
				break;
			}
			else
			{
				// downward search, always search for a higher priority node
				if (value <= _data[clidx])
				{
					parentidx = clidx;
				}
				else
				{
					value -= _data[clidx];
					parentidx = cridx;
				}
			}
		}

		targetidx = leafidx - _capacity + 1;
		leafValue = _data[leafidx];
	}

	double Sumtree::getMinP() const
	{
		double mini = _data[_capacity - 1];
		for (int i = _capacity - 1; i < _capacity; i++)
		{
			if (mini > _data[i])
				mini = _data[i];
		}
		return mini;
	}
}
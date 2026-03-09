#ifndef COORDSYS_H
#define COORDSYS_H

namespace coord
{
    class CartCoordSys
    {
    public:
        CartCoordSys(int dimension, int direction) : _dim(dimension), _dir(direction), idx{-1, -1, -1}
        {
            // sys = dimdir2Cord(dimension, direction);
            if (_dim == 1)
                idx[0] = _dir;
            else if (_dim == 2)
            {
                idx[0] = (_dir + 1) % 3;
                idx[1] = (_dir + 2) % 3;
                idx[2] = (_dir + 3) % 3;
            }
            else if (_dim == 3)
            {
                idx[0] = 0;
                idx[1] = 1;
                idx[2] = 2;
            }
        }
        CartCoordSys() : CartCoordSys(1, 0) {};

        CartCoordSys(const CartCoordSys &another) : _dim(another._dim), _dir(another._dir) //, sys(another.sys)
        {
            idx[0] = another.idx[0];
            idx[1] = another.idx[1];
            idx[2] = another.idx[2];
        }

        // CoordSystem sys;

        inline int operator[](int i) const
        {
#ifdef DEBUG
            if ((i >= _dim) || (i < 0))
            {
                throw(std::runtime_error("index out of bound in CordSystem, index = " + std::to_string(i)));
            }
#endif
            return idx[i];
        }

        inline int dim() const { return _dim; }
        inline int dir() const { return _dir; }

    private:
        int _dim;
        int _dir;
        int idx[3];

        // static CoordSystem dimdir2Cord(int dimension, int direction)
        // {
        //     if ((direction < 0) || (direction >= 3))
        //     {
        //         throw(std::runtime_error("Wrong direction in CordSys. Only 1, 2, or 3 is possible."));
        //     }
        //     if (dimension == 1)
        //     {
        //         return (CoordSystem)direction;
        //     }
        //     else if (dimension == 2)
        //     {
        //         return (CoordSystem)(direction * 10);
        //     }
        //     else if (dimension == 3)
        //     {
        //         return CoordSystem::xyz;
        //     }
        //     else
        //     {
        //         throw(std::runtime_error("Wrong dimension in CordSys. Only 1, 2, or 3 is possible."));
        //     }
        // }
    };

    // enum CoordSystem
    // {
    //     x = 0,
    //     y = 1,
    //     z = 2,
    //     yz = 10,
    //     zx = 20,
    //     xy = 30,
    //     xyz = 100,
    // };
}

#endif
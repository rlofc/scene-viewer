#ifndef GRID_H_3DXPAYRO
#define GRID_H_3DXPAYRO

#include "gameplay.h"
using namespace gameplay;

class Grid {
public:
    Grid( unsigned int lineCount );
    static Grid* create( unsigned int lineCount = 41 );

    operator Node*() { return _node; }
    void draw();

private:
    Node* _node;
};

#endif /* end of include guard: GRID_H_3DXPAYRO */



/*
 * Author Telnov Victor, v-telnov@yandex.ru
 */

#ifndef MTX_H
#define MTX_H

#include <QtCore>
#include <assert.h>
#include "tls.h"

struct Mtx {

    // поколонночно r1,...,r(cols)
    QVector<double> data;

    int cols = 0;
    int rows = 0;

    double at(int c, int r) const { return data.at(c*rows+r); }
    double& cell(int c, int r) { return data[c*rows+r]; }

    Mtx() {}
    void resize(int c, int r) { cols = c; rows = r; data.resize(c*r); }

    Mtx(int c, int r) : data(c*r), cols(c), rows(r) { assert(c>0&&r>0); }

    Mtx(int dim) : Mtx(dim,dim) {}

    Mtx(int c, int r, double v)
        : Mtx(c,r) { assert( c > 0 && r > 0 ); data.fill(v); }

    Mtx(int c, const QVector<double>& d)
        : data(d), cols(c), rows(d.size()/c) { assert( c > 0 && d.size()%c == 0 ); }

    Mtx(int c, int r, const double* ar)
        : Mtx(c,r) { memcpy(data.data(), ar, sizeof(*ar)*c*r); }

    static Mtx identity(int c, double v);
    void show() const;
    Mtx rot() const;
    Mtx append_col(double v) const;
    double dist(const Mtx& oth) const;
    Mtx mul(const Mtx& oth) const;
    Mtx operator*(const Mtx& oth) const { return mul(oth); }
    Mtx inv() const { return inv(*this); }
    static Mtx inv(const Mtx& m);

    static void test() {
        Mtx m(3, {3,1,6,5,-3,7,-2,2,-3});
        Mtx m2 = m.inv();
        Mtx m3 = m2 * m;
        m.show();
        m2.show();
        m3.show();
    }
};


#endif // MTX_H

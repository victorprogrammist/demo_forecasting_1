
/*
 * Author Telnov Victor, v-telnov@yandex.ru
 */

#include "tls.h"
#include "mtx.h"

Mtx Mtx::identity(int c, double v) {
    Mtx res(c,c);
    for (int i = 0; i < c; ++i) res.cell(i,i) = v;
    return res;
}

void Mtx::show() const {
    for (int r = 0; r < rows; ++r) {
        QString s;
        for (int c = 0; c < cols; ++c)
            s = s + (c==0?"":"   ") + QString::number(at(c,r),'f',6);
        qDebug() << s;
    }
}

Mtx Mtx::rot() const {
    if (rows == 1) return Mtx(1, data);
    if (cols == 1) return Mtx(rows, data);

    Mtx res(rows,cols);
    for (int c = 0; c < cols; ++c)
        for (int r = 0; r < rows; ++r)
            res.cell(r,c) = at(c,r);

    return res;
}

Mtx Mtx::append_col(double v) const {

    Mtx res(cols+1,rows);

    int c = cols*rows;

    memcpy(res.data.data(), data.data(), sizeof(double)*c);

    for (int i = 0; i < c; ++i)
        res.data[c+i] = v;

    return res;
}

double Mtx::dist(const Mtx& oth) const {
    assert( cols == oth.cols && rows == oth.rows );
    double su = 0;
    for (int i = 0; i < data.size(); ++i)
        su += std::abs( data.at(i) - oth.data.at(i) );
    return su;
}

Mtx Mtx::mul(const Mtx& oth) const {

    assert( cols == oth.rows );

    Mtx res(oth.cols, rows);
    for (int c = 0; c < oth.cols; ++c)
        for (int r = 0; r < rows; ++r) {
            double su = 0;
            for (int m = 0; m < cols; ++m)
                su += at(m, r) * oth.at(c, m);
            res.cell(c, r) = su;
        }

    return res;
}

Mtx Mtx::inv(const Mtx& m) {
    assert( m.cols == m.rows );

    int dim = m.cols;
    Mtx A = m;
    Mtx I = identity(dim, 1);

    for (int z = 0; z < dim; ++z) {
        double k = A.at(z,z);
        //A.cell(z,z) = 1.0;

        for (int c = z+1; c < dim; ++c)
            A.cell(c,z) /= k;

        for (int c = 0; c <= z; ++c)
            I.cell(c,z) /= k;

        for (int r = z+1; r < dim; ++r) {
            k = A.at(z,r);
            //A.cell(z,r) = 0;

            for (int c = z+1; c < dim; ++c)
                A.cell(c,r) -= A.at(c,z) * k;
            for (int c = 0; c <= z; ++c)
                I.cell(c,r) -= I.at(c,z) * k;
        }
    }

    for (int z = dim-1; z > 0; --z) {

        for (int r = z-1; r >= 0; --r) {
            double k = A.at(z,r);
            //A.cell(z,r) = 0;

            for (int c = 0; c < dim; ++c)
                I.cell(c, r) -= I.at(c, z) * k;
        }
    }

    return I;
}

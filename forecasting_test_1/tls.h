#ifndef TLS_H
#define TLS_H

#include <QtCore>
#include <assert.h>

using std::pair;

[[noreturn]] inline void do_throw(const QString& msg) {
    qDebug() << msg;
    throw msg;
}

inline QString procent(double v) {
    return QString::number(v*100, 'f', 3) + "%";
}

template <class A> double dbl(A v) { return static_cast<double>(v); }

template <class A, class B> double safe_div(A v1, B v2) { return dbl(v1) / dbl(v2); }

QStringList get_file_lines(const QString& file_name);
QStringList get_file_col(const QString& file_name, const QString& col, const QString& spl);
QVector<double> get_file_col_num(const QString& file_name, const QString& col, const QString& spl);

double corr_pow2(const double* v1, const double* v2, int cnt);
double sqrt_sign(double v);

#endif // TLS_H

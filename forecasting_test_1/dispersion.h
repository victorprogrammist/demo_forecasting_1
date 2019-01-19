
/*
 * Author Telnov Victor, v-telnov@yandex.ru
 */

#ifndef DISPERSION_H
#define DISPERSION_H

#include <cmath>
#include <assert.h>

struct Dispersion {

    double su = 0;
    double su_pow2 = 0;
    double su_w = 0;

    double ma = 0;
    double mi = 0;

    int cnt_add_value = 0;

    void clear();
    void add_value(double v) { add_value(v, 1.0); }
    void add_value(double v, double w);

    // !!! не корректирует ma & mi на случай вычитания этих крайних значений
    void   subtraction_value(double v, double w = 1.0);

    double unbiased_sample_variance() const;
    // !!! использует unbiased_sample_variance => делитель cnt-1
    double stddev() const { return pow(unbiased_sample_variance(),0.5); }
    double stddev_biased() const { return pow(sample_variance(),0.5); }
    double mean() const;
    double mean_2() const;

    double mi_2s() const { return mean() - stddev() * 2.0; }
    double ma_2s() const { return mean() + stddev() * 2.0; }

    double sample_variance() const;

    void add_dispersion(Dispersion& oth, double koef = 1.0);

    void substraction_dispersion(Dispersion& oth) { add_dispersion(oth, -1.0); }
};


inline void Dispersion::add_dispersion(Dispersion& oth, double koef) {
    su += oth.su * koef;
    su_pow2 += oth.su_pow2 * koef;
    su_w += oth.su_w * koef;

    cnt_add_value += oth.cnt_add_value;

    if (koef > 0.0) {
        if (oth.ma > ma) ma = oth.ma;
        if (oth.mi < mi) mi = oth.mi;
    }
}

inline void Dispersion::clear() {
    su = 0;
    su_pow2 = 0;
    su_w = 0.0;
    ma = 0;
    mi = 0;
}

inline void Dispersion::add_value(double v, double w) {
    assert( w >= 0.0 );

    if (su_w == 0.0) mi = ma = v;
    else if (v < mi) mi = v;
    else if (v > ma) ma = v;

    su += v * w;
    su_pow2 += v * v * w;
    su_w += w;
    cnt_add_value++;
}

// !!! не корректирует ma & mi на случай вычитания этих крайних значений
inline void Dispersion::subtraction_value(double v, double w) {
    assert( su_w >= w && w > 0.0 );

    su -= v * w;
    su_pow2 -= v * v * w;
    su_w -= w;
    cnt_add_value--;
}

inline double Dispersion::unbiased_sample_variance() const {
    assert( su_w > 1.0 );
    return (su_pow2 - su * su / su_w) / (su_w - 1.0);
}

inline double Dispersion::mean() const {
    assert( su_w > 0.0 );
    return su / su_w;
}

inline double Dispersion::mean_2() const {
    assert( su_w > 0.0 );
    return pow(su_pow2 / su_w, 0.5);
}

inline double Dispersion::sample_variance() const {
    assert( su_w > 0.0 );
    return (su_pow2 - su * su / su_w) / su_w;
}

#endif // DISPERSION_H

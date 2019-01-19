
#ifndef ORD_PACK_H
#define ORD_PACK_H

#include <QtCore>

struct OrdItem {
    double koef;
    int    i_pos;
};

struct OrdPack {

    int cnt_all_values = 0;

    const int ma_count_for_pack = 50;

    OrdPack(int ma_c) : ma_count_for_pack(ma_c) {}

    std::multimap<double,int> mmap_ord;
    double mi_koef = 0;

    void add_value(double koef, int i_pos);
    QVector<OrdItem> get_values();
    QVector<int> get_values_only_positions();
};

inline void OrdPack::add_value(double koef, int i_pos) {

    if (std::isfinite(koef)==false) return;

    ++cnt_all_values;

    if (koef <= 0.0) return;

    if (mmap_ord.size() < ma_count_for_pack) {

        if (mmap_ord.size()==0)
            mi_koef = koef;

        mi_koef = std::min(mi_koef, koef);
        mmap_ord.insert({-koef,i_pos});

    } else if (koef > mi_koef) {

        mmap_ord.insert({-koef,i_pos});

        while (mmap_ord.size() > ma_count_for_pack)
            mmap_ord.erase(--mmap_ord.end());

        mi_koef = -(--mmap_ord.end())->first;
    }
}

inline QVector<OrdItem> OrdPack::get_values() {

    QVector<OrdItem> res;
    for (auto p : mmap_ord)
        res.append({-p.first,p.second});

    return res;
}

inline QVector<int> OrdPack::get_values_only_positions() {

    QVector<int> res;
    for (auto p : mmap_ord)
        res.append(p.second);

    return res;
}

#endif // ORD_PACK_H

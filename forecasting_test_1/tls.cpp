
/*
 * Author Telnov Victor, v-telnov@yandex.ru
 */

#include "tls.h"

double sqrt_sign(double v) {
    if (v >= 0) return pow(v, 0.5);
    return -pow(-v, 0.5);
}

double corr_pow2(const double* v1, const double* v2, int cnt) {

    double su_x_y = 0;
    double su_x = 0;
    double su_y = 0;
    double su_x_2 = 0;
    double su_y_2 = 0;

    for (int i = 0; i < cnt; ++i) {
        double x = v1[i];
        double y = v2[i];

        su_x_y += x * y;
        su_x += x;
        su_y += y;
        su_x_2 += x*x;
        su_y_2 += y*y;
    }

    double d1 = cnt * su_x_y - su_x * su_y;
    double d2 = cnt * su_x_2 - su_x * su_x;
    double d3 = cnt * su_y_2 - su_y * su_y;

    if (d2 == 0.0 || d3 == 0.0)
        return -100;

    if (d1 >= 0)
        return safe_div(d1*d1, d2*d3);
    else
        return safe_div(-d1*d1, d2*d3);
}

QByteArray file_read_raw(const QString& fn) {
    QFile file(fn);

    if (file.exists() == false) do_throw("file not found");

    if (file.open(QIODevice::ReadOnly) == false)
        throw "file not openned";

    return file.read(file.size());
}

QStringList get_file_lines(const QString& file_name) {
    auto ba = file_read_raw(file_name);
    QString s = QTextCodec::codecForName("cp1251")->toUnicode(ba);

    QRegExp spl("\r?\n");
    return s.split(spl, QString::SkipEmptyParts);
}

QStringList get_file_col(const QString& file_name, const QString& col, const QString& spl) {

    QStringList list = get_file_lines(file_name);

    QStringList res;

    if (list.isEmpty()) return res;

    int i_col = list.at(0).split(spl).indexOf(col);
    if (i_col < 0) return res;

    for (int i = 1; i < list.size()-1; ++i)
        res.append(list.at(i).split(spl).at(i_col));

    return res;
}

QVector<double> get_file_col_num(const QString& file_name, const QString& col, const QString& spl) {

    QStringList ss = get_file_col(file_name, col, spl);
    QVector<double> res(ss.size());

    for (int i = 0; i < ss.size(); ++i)
        res[i] = ss.at(i).toDouble();

    return res;
}


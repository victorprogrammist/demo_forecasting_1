
/*
 * Author Telnov Victor, v-telnov@yandex.ru
 *
 * Данная программа является демонстрационной
 * к статье https://habr.com/ru/post/435590/
 *
 * Исходная идея взята из статьи
 * https://habr.com/post/267035/
 *
 */


#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "tls.h"
#include "dispersion.h"
#include "mtx.h"
#include "ord_pack.h"

using std::pair;

pair<double,double> get_a_b(
        QVector<double>& ar_cotir,
        int i_forecast,
        int i_old,
        int len_same) {
    // метод расчета коэффициентов переноса,
    //   принцип которого взят из статьи https://habr.com/post/267035/

    Mtx mtx_y(1, len_same, ar_cotir.data()+i_forecast-len_same+1);
    Mtx mtx_x(1, len_same, ar_cotir.data()+i_old-len_same+1);
    Mtx mtx_x_1 = mtx_x.append_col(1);
    Mtx mtx_xx = mtx_x_1.rot();
    Mtx ab = (mtx_xx * mtx_x_1).inv() * (mtx_xx * mtx_y);
    return {ab.at(0,0),ab.at(0,1)};
}

// расчет прогноза одного значения,
//   позиция подобия уже определена,
//   и здесь только расчет коэффициентов переноса
//   из этой позиции и применения их к одной точке
double calc_forecast(
        QVector<double>& ar_cotir,
        int i_forecast,
        int i_old,
        int len_same,
        int i_pos_forecast) {

    // многократный не рациональный вызов расчета коэффициента,
    //   но код демонстрационный, поэтому не важно
    pair<double,double> ab = get_a_b(ar_cotir, i_forecast, i_old, len_same);

    double v_forecast = ar_cotir.at(i_old+1+i_pos_forecast) * ab.first + ab.second;

    return v_forecast;
}

// обсчет ошибки прогноза по одной позиции на несколько точек вперед
double calc_pos(
        QVector<double>& ar_cotir,
        int i_forecast,
        int i_old,
        int len_same,
        int len_forecast) {

    Dispersion disp_mape;

    for (int i = 0; i < len_forecast; ++i) {
        double v_fact = ar_cotir.at(i_forecast+1+i);
        double v_forecast = calc_forecast(ar_cotir, i_forecast, i_old, len_same, i);
        disp_mape.add_value(std::abs(v_forecast - v_fact)/v_fact);
    }

    return disp_mape.mean();
}

// обсчет ошибки прогноза по массиву исходных точек - выборка лучших или случайная выборка
// возвращает отклонение факта от усредненного прогноза и его стандартное отклонение,
//    а также средний прогноз и стандартное отклонение без учета факта
pair<double,double> calc_pos_by_array(
        QVector<double>& ar_cotir,
        int i_forecast,
        QVector<int>& ar_pos,
        int len_same,
        int len_forecast,
        double& res_avg_forecast,
        double& res_stddev_forecast) {

    Dispersion disp_avg_mape;
    Dispersion disp_dev_mape;

    Dispersion disp_avg_forecast;
    Dispersion disp_stddev_forecast;

    for (int i_pos_forecast = 0; i_pos_forecast < len_forecast; ++i_pos_forecast) {
        double v_fact = ar_cotir.at(i_forecast+1+i_pos_forecast);

        // расчет прогноза усредненно по всей пачке
        Dispersion disp_forecast;
        Dispersion disp_mape;
        for (int i_pos : ar_pos) {

            double v_forecast = calc_forecast(ar_cotir, i_forecast, i_pos, len_same, i_pos_forecast);
            disp_forecast.add_value(v_forecast);

            double v_mape = std::abs(v_forecast - v_fact) / v_fact;
            disp_mape.add_value(v_mape);
        }

        double v_avg_forecast = disp_forecast.mean();

        double v_avg_mape = std::abs(v_avg_forecast - v_fact) / v_fact;

        disp_avg_mape.add_value(v_avg_mape);
        disp_dev_mape.add_value(disp_mape.stddev());

        disp_avg_forecast.add_value(disp_forecast.mean());
        disp_stddev_forecast.add_value(disp_forecast.stddev());
    }

    res_avg_forecast = disp_avg_forecast.mean();
    res_stddev_forecast = disp_stddev_forecast.mean();

    return {disp_avg_mape.mean(),disp_dev_mape.mean()};
}

// Основная процедура теста.
// Смысл расчета во многом похож расчету исходной статьи
//  https://habr.com/post/267035/
//  и с теми же параметрами расчета.
// Здесь же добавлено то, что все это считается по пачке лучших
//  которая накапливается в объекте OrdPack pack(cnt_best),
//  и на основе этого проверяется насколько закономерен этот прогноз.
void MainWindow::to_do_test() {

    QString file_cotir = ui->ed_file_data->text();
    QString tm_for_forecast = ui->ed_start_date_time->text();

    // количество в пачке лучших подобий
    int cnt_best = ui->ed_count_best->text().toInt();

    // считывание данных из файла котировок
    QVector<double> ar_cotir = get_file_col_num(file_cotir, "Value", ",");
    QStringList ar_tm = get_file_col(file_cotir, "DateTime", ",");

    // определение позиции прогноза по временной метке
    int i_forecast = ar_tm.indexOf(tm_for_forecast);
    if (i_forecast < 0) do_throw("Дата/время не найдено: "+tm_for_forecast);

    int len_stp = 24;
    int len_forecast = 24;
    int len_corr = 144;
    OrdPack pack(cnt_best);
    QVector<int> ar_all_pos;

    // расчет корреляций и отбор лучших
    int i_pos = i_forecast - len_stp*2;
    for (; i_pos >= len_corr+len_stp; i_pos -= len_stp) {
        const double* ar_now = ar_cotir.constData() + i_forecast - len_corr + 1;
        const double* ar_old = ar_cotir.constData() + i_pos - len_corr + 1;
        double cr = sqrt_sign(corr_pow2(ar_now, ar_old, len_corr));
        // осуществляет отбор и сохранение лучших подобий
        pack.add_value(cr, i_pos);
        ar_all_pos.append(i_pos);
    }

    // преобразование множества лучших подобий к простому списку
    //   в порядке убывания корреляции
    QVector<OrdItem> lst_best = pack.get_values();
    OrdItem& best = lst_best[0];
    OrdItem& last_best = lst_best[lst_best.size()-1];

    int i_best = best.i_pos;

    // расчет прогноза по одной самой лучшей точке
    pair<double,double> ab = get_a_b(ar_cotir, i_forecast, i_best, len_corr);
    double mape = calc_pos(ar_cotir, i_forecast, i_best, len_corr, len_forecast);

    qDebug() << "расчет прогноза на момент" << tm_for_forecast << "позиция" << i_forecast;
    qDebug() << "всего проверено значений на подобие" << pack.cnt_all_values;
    qDebug() << "самая лучшая корреляция" << best.koef << "позиция" << best.i_pos;
    qDebug() << "коэффициенты переноса alpha(1/2)" << ab.first << ab.second;
    qDebug() << "ошибка прогноза от факта mape" << procent(mape);

    qDebug() << "отобрано лучших" << lst_best.size();
    qDebug() << "самая худшая из лучших, корр" << last_best.koef << "позиция" << last_best.i_pos;

    // расчет прогноза для каждого лучшего отрезка поотдельности
    qDebug() << "============================";
    for (int i = 0; i < lst_best.size(); ++i) {

        OrdItem& ord = lst_best[i];

        mape = calc_pos(ar_cotir, i_forecast, ord.i_pos, len_corr, len_forecast);

        qDebug() << i
                 << "corr" << ord.koef
                 << "pos" << ord.i_pos
                 << "mape" << procent(mape);
    }

    // расчет усредненного прогноза по всей пачке

    QVector<int> ar_best_pos = pack.get_values_only_positions();

    double avg_forecast_pack;
    double stddev_forecast_pack;
    pair<double,double> mape_pack = calc_pos_by_array(
                ar_cotir, i_forecast, ar_best_pos, len_corr, len_forecast,
                avg_forecast_pack, stddev_forecast_pack);

    // в массиве ar_all_pos собраны все позиции которые использовались
    //   для расчета корреляций, и их считаем случайной выборкой
    double avg_forecast_rand;
    double stddev_forecast_rand;
    pair<double,double> mape_rand = calc_pos_by_array(
                ar_cotir, i_forecast, ar_all_pos, len_corr, len_forecast,
                avg_forecast_rand, stddev_forecast_rand);

    qDebug() << "====================================";
    qDebug() << "результат усредненный по лучшим корреляциям mape" << procent(mape_pack.first);
    qDebug() << "стандартное отклонение mape" << procent(mape_pack.second);
    qDebug() << "сам прогноз" << avg_forecast_pack << "станд.отклонение" << stddev_forecast_pack
             << "(основа koef_forecast)";
    qDebug() << "====================================";
    qDebug() << "расчет прогноза по случайной выборке, mape" << procent(mape_rand.first);
    qDebug() << "стандартное отклонение mape" << procent(mape_rand.second);
    qDebug() << "сам прогноз" << avg_forecast_rand << "станд.отклонение" << stddev_forecast_rand
             << "(основа koef_forecast)";
    qDebug() << "====================================";
    qDebug() << "коэффициент теоретической эффективности прогнозирующего алгоритма";
    qDebug() << "для текущей точки, koef_forecast"
             << 1.0 - stddev_forecast_pack / stddev_forecast_rand;
}

//************************************************************************************************
// далее вспомогательные функции

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    original_message_handler = qInstallMessageHandler(myMessageOutput);

    QString file_cotir = "/ai/eurusd/emmsp.txt";

    if (QFile(file_cotir).exists()==false) {
        QString sp = QDir::separator();

        QString s = qApp->applicationDirPath();
        if (s.right(1) != "/" && s.right(1) != "\\")
            s = s + sp;

        file_cotir = s.replace("/", sp) + "data" + sp + "emmsp.txt";
    }

    ui->ed_file_data->setText(file_cotir);
    ui->ed_start_date_time->setText("9/1/2012 23:00");
    ui->ed_count_best->setText("100");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    ui->txt_log->clear();
    try { to_do_test(); } catch (...) {}
}

void MainWindow::myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg) {

    MainWindow* w = nullptr;
    for(QWidget* pWidget : QApplication::topLevelWidgets()) {
        w = qobject_cast<MainWindow*>(pWidget);
        if (w) break;
    }

    QString s = msg;
    s = s.replace("\"", "");

    bool has_comma = w->ui->checkBox_UseComma->checkState() == Qt::Checked;
    if (has_comma)
        s = s.replace(".", ",");
    else
        s = s.replace(",", ".");

    w->ui->txt_log->appendPlainText(s);
    w->ui->txt_log->moveCursor(QTextCursor::End);

    w->original_message_handler(type, context, msg);
}

